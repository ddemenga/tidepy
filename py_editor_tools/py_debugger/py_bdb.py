import os,sys
import fnmatch
import functools
import linecache
import py_cmd as cmd


from inspect import CO_GENERATOR, CO_COROUTINE, CO_ASYNC_GENERATOR

GENERATOR_AND_COROUTINE_FLAGS = CO_GENERATOR | CO_COROUTINE | CO_ASYNC_GENERATOR


class BdbQuit(Exception):
    """
        Exception to give up completely.
    """



class Breakpoint:



    def __init__(self, filename, lineno, enabled = True, cond=None):
        self.number = id
        self.filename = filename
        self.lineno = lineno
        self.cond = cond
        self.enabled = enabled


    def __str__(self):
        return f"filename:{self.filename},lineno={self.lineno},cond ={self.cond} , enabled = {self.enabled}"

class Breakpoints:


    def __init__(self):
        self.breaks = {}
        pass

    @property
    def is_empty(self):
        return len(self.breaks) == 0

    def set_break(self, filename : str , lineno : int , enabled :bool = True , cond :callable  = None ) -> None:
        """Set a new breakpoint for filename:lineno.

        If lineno doesn't exist for the filename, return an error message.
        The filename should be in canonical form.
        """

        breaks =  self.breaks.setdefault(filename, {} )
        if lineno in breaks:
            breaks[lineno].cond = cond
            breaks[lineno].enabled = enabled
        else:
            breaks[lineno] = Breakpoint( filename , lineno , enabled=enabled, cond=cond )



    def get_break(self , filename : str  , lineno : int ) -> Breakpoint:
        if filename not in self.breaks:
            return None
        breaks = self.breaks[filename]
        if lineno in breaks:
            return breaks[lineno]
        return None

    def del_break(self , filename , lineno ) -> bool:

        if filename not in self.breaks:
            return False
        breaks = self.breaks[filename]
        if lineno in breaks:
            del breaks[lineno]
            return True
        return False

    def clear(self):
        self.breaks = {}

    def set_break_enabled(self , filename : str  , lineno : int, value : bool ) -> bool:
        b = self.get_break(filename, lineno)
        if b is not None :
            b.enabled = value

    def has_breaks( self, filename ):
        return filename in self.breaks and len(self.breaks[filename])

    def break_here(self, filename:  str  , lineno : int , apply_cond  = None ) -> bool :
        """Return True if there is an effective breakpoint for this line.

        Check for line or function breakpoint and if in effect.
        Delete temporary breakpoints if effective() says to.
        """

        b = self.get_break(filename, lineno)
        if b is None :
            return False

        if not b.enabled:
            return False

        if b.cond is not None :
            if  apply_cond(b.cond):
                return True

            return False
        return True




    def print_breaks(self):
        for b in self.breaks:
            bs = self.breaks[b]
            for lno in bs :
                print ( bs[lno] )






class Bdb( Breakpoints , cmd.Cmd  ):

    STEP_INTO = 0
    NEXT = 1
    STEP_OUT = 2
    CONTINUE = 3
    RUN = 4

    decode_stop_mode = {0:"step",1:"next",2:"step out",3:"continue",4:"run"}

    def __init__( self, debugger_hook, std_in, std_dbg ):
        self.std_in = std_in
        self.std_dbg = std_dbg

        Breakpoints.__init__(self)
        cmd.Cmd.__init__(self, debugger_hook, completekey='tab', stdin= std_in, stdout=std_dbg)

        self.cache = set()

        self.frame = None
        self.trace_frame = None
        self.return_frame = None
        self.stop_frame = None
        self.stop_mode = Bdb.CONTINUE

        self.dirname = os.path.dirname(__file__)
        self.__debugger_names__ = [ __file__,\
                                    os.path.join ( self.dirname , 'py_debugger_main.py' ) ,\
                                    os.path.join ( self.dirname , 'py_cmd.py' ) ,\
                                    os.path.join ( self.dirname , 'py_dbg.py' )]




    # UTILITIES #######################################################à
    @functools.cache
    def canonic(self, filename):

        if filename == "<" + filename[1:-1] + ">":
            return filename
        canonic = os.path.abspath(filename)
        return os.path.normcase(canonic)


    def get_line_info( self,frame ):
        return self.format_stack_entry((frame, frame.f_lineno))

    def get_filename(self ,frame ):
        return self.canonic(frame.f_code.co_filename)

    def get_line_source_code(self, frame, lineno ):
        s = ""
        if frame.f_code.co_name:
            s += frame.f_code.co_name

        else:
            s += "<lambda> "
        return s , linecache.getline(self.get_filename(frame), lineno, frame.f_globals).strip()

    @functools.cache
    def belongs_to( self, modulename: str  , pattern : str ):
        return fnmatch.fnmatch(modulename, pattern) if modulename else False

    def format_stack_entry(self, frame_lineno, lprefix=': '):
        """Return a string with information about a stack entry.

        The stack entry frame_lineno is a (frame, lineno) tuple.  The
        return string contains the canonical filename, the function name
        or '<lambda>', the input arguments, the return value, and the
        line of code (if it exists).

        """
        import linecache, reprlib
        frame, lineno = frame_lineno
        filename = self.canonic(frame.f_code.co_filename)
        s = '%s(%r)' % (filename, lineno)
        if frame.f_code.co_name:
            s += frame.f_code.co_name
        else:
            s += "<lambda>"
        s += '()'
        if '__return__' in frame.f_locals:
            rv = frame.f_locals['__return__']
            s += '->'
            s += reprlib.repr(rv)
        line = linecache.getline(filename, lineno, frame.f_globals)
        if line:
            s += lprefix + line.strip()
        return s

    # END UTILITIES #######################################################à



    def __set_break__(self, filename, lineno, enabled= True, cond=None ):

        filename = self.canonic(filename)
        line = linecache.getline(filename, lineno)
        line = line.strip()
        if line is None or line == '':
            return False
        Breakpoints.set_break(self, filename, lineno, enabled=enabled , cond=cond)
        return True




    def trace(self, frame, event, arg):

        filename = frame.f_code.co_filename
        module_name = frame.f_globals.get('__name__')

        self.frame  = frame


        if event == 'call':

            # no trace for the debugger
            if filename in self.__debugger_names__:
                return None

            if self.stop_mode == Bdb.STEP_INTO :
                self.stop_mode = Bdb.NEXT
                self.stop_frame = self.frame

                return self.trace

            elif self.has_breaks( filename ):

                if frame.f_code.co_flags & GENERATOR_AND_COROUTINE_FLAGS:
                    #print(f"No generators {filename}[ {frame.f_lineno} ] :  {frame.f_code.co_flags}")
                    return None

                return self.trace

            return None


        elif event == 'line':


            if self.frame == self.stop_frame and self.stop_mode <= Bdb.NEXT :

                #print ( f"module_name={module_name} {filename}[ {frame.f_lineno} ]" )
                #print( f"stop_mode={self.decode_stop_mode[self.stop_mode]}, breaks={self.breaks}")
                #print( linecache.getline(filename, frame.f_lineno ))
                self.user_line(frame, arg)

                return self.trace

            elif self.break_here( filename, frame.f_lineno ) :
                self.stop_frame = self.frame

                #print ( f"break at :: module_name={module_name} {filename}[ {frame.f_lineno} ]" )
                #print( f"stop_mode={self.decode_stop_mode[self.stop_mode]}, breaks={self.breaks}")
                #print( linecache.getline(filename, frame.f_lineno ))
                self.user_line(frame,arg)

                return self.trace

            return None

        elif event == 'return':

            if frame == self.return_frame:

                if self.stop_mode == Bdb.STEP_OUT:
                    self.stop_mode = Bdb.NEXT


                #if frame.f_back and frame.f_back.f_trace:
                #    print( f"return to module_name= {frame.f_back} ]")

            if self.stop_mode < self.CONTINUE :
                self.stop_frame = frame.f_back

            return self.trace

        elif event == 'exception':
            return self.trace




    def user_line( self, frame , arg ):
        """
            Subclass implementation
        """
        raise NotImplementedError


    # FLOW CONTROL #############################################################

    def sigint_handler(self, signum, frame):
        #if self.allow_kbdint:
        #    raise KeyboardInterrupt
        print("Program interrupted. (Use 'cont' to resume).", file = sys.stderr )
        self.frame = frame
        self.do_step_into(None)



    def do_exit(self,arg):
        sys.settrace(None)
        raise BdbQuit
    do_e = do_exit

    def do_continue(self,arg):
        self.stop_mode = Bdb.CONTINUE
        self.stop_frame = None
        return True
    do_c =  do_continue

    def do_run(self,arg):
        import sys
        self.stop_mode = Bdb.RUN
        self.stop_frame = None
        sys.settrace(None)
        return True
    do_r = do_run

    def do_step_out(self,arg):
        self.stop_mode = Bdb.STEP_OUT
        if not self.return_frame :
            self.return_frame = self.frame
        self.stop_frame = None
        return True
    do_o = do_step_out

    def do_step_into(self,arg):
        self.stop_mode = Bdb.STEP_INTO
        self.return_frame = None
        return True
    do_s = do_step_into

    def do_next(self,arg):
        self.stop_mode = Bdb.NEXT
        self.return_frame = None
        return True
    do_n = do_next

    # END FLOW CONTROL #############################################################



    def run(self, run_type, cmd, filename , globals=None, locals=None):
        """Debug a statement executed via the exec() function.

        globals defaults to __main__.dict; locals defaults to globals.
        """
        if globals is None:
            import __main__
            globals = __main__.__dict__
        if locals is None:
            locals = globals

        if isinstance(cmd, str):
            cmd = compile(cmd, filename , "exec")#"<string>", "exec")

        if run_type == "debug":
            sys.settrace(self.trace)

        try:
            exec(cmd, globals, locals)

        except BdbQuit:
            pass

        finally:
            sys.settrace(None)



    def runscript(self,run_type, filename):
        # The script has to run in __main__ namespace (or imports from
        # __main__ will break).
        #
        # So we clear up the __main__ and set several special variables
        # (this gets rid of pdb's globals and cleans old variables on restarts).
        import __main__
        import builtins
        import io


        # When bdb sets tracing, a number of call and line events happens
        # BEFORE debugger even reaches user's code (and the exact sequence of
        # events depends on python version). So we take special measures to
        # avoid stopping before we reach the main script (see user_line and
        # user_call for details).
        #self._wait_for_mainpyfile = True
        mainpyfile = self.canonic(filename)

        try:
            with io.open_code(filename) as fp:
                statement = "exec(compile(%r, %r, 'exec'))" % (fp.read(), mainpyfile)
                if os.path.dirname(mainpyfile) not in sys.path:
                    sys.path.insert(0, os.path.dirname(mainpyfile) )

                globals_ = {'__name__': '__main__', '__doc__': '', '__package__': '', '__loader__': None, '__spec__': None, '__file__': mainpyfile, '__builtins__' : builtins  }
                #print( f'run {mainpyfile} with breaks={self.breaks}')
                self.run(run_type,statement,mainpyfile,globals_)

        except Exception as e :
            self.print_exception(e)

        finally :
            del sys.path[0]

