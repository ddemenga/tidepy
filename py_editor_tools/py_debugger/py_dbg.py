import py_bdb as bdb


import threading
import traceback
import sys
import inspect
import os
import io
import json





class Json :

    def __init__(self, reason ):
        self.buf = io.StringIO()
        self.data = {'reason':reason }

    def add_element(self, type_name  , **kwds):

        self.data[type_name] = {}
        value = self.data[type_name]
        for k in kwds.keys() :
            value[k] = kwds[k]

    @staticmethod
    def print_element( reason, type_name, **kwds ):

        buf = io.StringIO()

        data = {'reason':reason }
        data[type_name] = {}
        value = data[type_name]

        for k in kwds.keys() :
            value[k] = kwds[k]

        json.dump( data , buf , separators=(',', ':'))
        print( buf.getvalue() , file = sys.stddbg )

    @staticmethod
    def print_exception( reason, type_name, **kwds ):

        buf = io.StringIO()

        data = {'reason':reason }
        data[type_name] = {}
        value = data[type_name]

        for k in kwds.keys() :
            value[k] = kwds[k]

        json.dump( data , buf , separators=(',', ':'))
        print( buf.getvalue() , file = sys.stderr )


    def print(self, file = None ):


        buf = io.StringIO()
        json.dump( self.data , buf , separators=(',', ':'))
        if file is None:
            print( buf.getvalue() , file = sys.stddbg )
        else:
            print( buf.getvalue() , file = file )








class Dbg( bdb.Bdb  ):
    """
        The debugger class.

        It is a simplified and modified version of pdb.Pdb, cmd.Cmd, bdb.Bdb.
        It wraps input-output from and to the debugged program to json objects.

    """

    _previous_sigint_handler = None

    def __init__(self,debugger_hook):

        self.skip = set()

        bdb.Bdb.__init__(self, debugger_hook , sys.stdin, sys.stddbg )


        # used only to get the frame for refreshing watches on user request
        self.frames_stack = {}
        self.watched_vars = {}

        import signal
        try:
            self._previous_sigint_handler = signal.signal(signal.SIGINT, self.sigint_handler)
        except ValueError:
            # ValueError happens when do_continue() is invoked from
            # a non-main thread in which case we just continue without
            # SIGINT set. Would printing a message here (once) make
            # sense?
            pass


    def add_skip_module_pattern(self, pattern ):
        if pattern not in self.skip :
            self.skip.add( pattern )


    def print_exception( self , e ) :
        buf = io.StringIO()
        traceback.print_exception(e, file = buf )
        Json.print_exception( "exception" , "stack" , value = buf.getvalue() )


    def print_stack_frames(self):

        d = Json("frames")

        self.frames_stack.clear()
        frames = []
        for i,info in enumerate( inspect.stack(0) ):
            filename = self.get_filename( info.frame )
            if not filename.startswith('<') and info.filename not in  self.__debugger_names__ :
                func , source_code  = self.get_line_source_code( info.frame , info.lineno )

                frame = {}
                frame['pid'] = threading.get_native_id()
                frame['id'] = threading.get_ident()
                frame['filename'] = info.filename #self.get_filename(info.frame)
                frame['lineno'] = info.lineno #frame.f_lineno
                #frame['text'] =  info.code_context[info.index] if info.code_context else source_code
                frame['func'] = info.code_context[info.index] if info.code_context else func
                frame['active'] = "no"
                frame['index'] = i

                #print(f"frame {i} {frame['filename']} {frame['lineno']}", file = sys.stddbg )
                if self.frame is not None:
                    cur_filename  =self.get_filename(self.frame)

                    if self.frame.f_lineno == info.lineno and filename == cur_filename :
                        frame['active']= "yes"

                self.frames_stack[i] = info
                frames.append(frame)


        d.data[ "stack" ] = frames
        d.print()


    def user_line(self, frame,arg):
        """Called when we stop or break at a line."""

        Json.print_element("break", "stop" , filename=self.get_filename(frame), lineno= f"{frame.f_lineno}" , text = f"{self.get_line_info(frame)}" )

        self.display_vars()
        self.print_stack_frames()
        self.cmdloop("line dbg>")







    # WATCHES ######################################################################################

    def __getval__(self, arg):
        try:
            return eval(arg, self.frame.f_globals, self.frame.f_locals)
        except:
            exc_info = sys.exc_info()[:2]
            Json.print_exception( "exception", "error" , value= traceback.format_exception_only(*exc_info)[-1].strip() )
            raise

    def __getval_except__(self, arg, frame=None):
        try:
            if frame is None:
                return eval(arg, self.frame.f_globals, self.frame.f_locals)
            else:
                return eval(arg, frame.f_globals, frame.f_locals)
        except:
            #exc_info = sys.exc_info()[:2]
            #err = traceback.format_exception_only(*exc_info)[-1].strip()
            return '<no such value>'

    def do_display_expression(self, arg ):
        if arg is not None and len(arg):
            Json.print_element("display_expr","expr", value= str(self.__getval_except__(arg)) )

    do_de = do_display_expression

    def display_vars(self):

        vars_out = []
        for id,name in self.watched_vars.items():

            #print( f" try get var {name}" )
            val = self.__getval_except__(name)
            try:
                vars_out.append(  { 'id' : id , 'name':name , 'type':f"{type(val)}", 'value' :  str(val) } )
            except :
                vars_out.append(  { 'id' : id , 'name':name , 'type':f"{type(val)}", 'value' :  '<no such value>' } )

        d = Json("display_batch")
        d.data["value"] = vars_out
        #print( f" {d.data}", file = sys.stddbg )
        d.print()


    def display_vars_at_frame(self,frame):

        vars_out = []
        for id,name in self.watched_vars.items():

            val = self.__getval_except__(name,frame)
            try:
                vars_out.append(  { 'id' : id , 'name':name , 'type':f"{type(val)}", 'value' :  str(val) } )
            except :
                vars_out.append(  { 'id' : id , 'name':name , 'type':f"{type(val)}", 'value' :  '<no such value>' } )
        d = Json("display_batch")
        d.data["value"] = vars_out
        #print( f" {d.data}", file = sys.stddbg )
        d.print()

    def do_display_batch_at_frame(self, args ):

        try:
            iframe = int(args)
            #print(f"frames filtered = {self.no_frame_for}", file = sys.stddbg )

            for i in self.frames_stack:
                if iframe == i:
                    info =self.frames_stack[i]
                    #print(f"read vars from frame {i} ,{info.filename}", file=sys.stddbg)
                    self.display_vars_at_frame(info.frame)
                    break


        except Exception as e:
            self.print_exception(e)


    def do_add_watch(self, args ):

        var_name, var_id = args.split()
        self.watched_vars[ int(var_id) ] = var_name
        self.display_vars()

    def do_add_watch_batch(self, args ):
        import json

        args = io.StringIO(args)

        for var in json.load( args ):
            self.watched_vars[var['id']] = var['name']
        self.display_vars()


    def do_del_watch(self, var_id ):

        var_id = int(var_id)
        if var_id in self.watched_vars:
            del self.watched_vars[var_id];
            self.display_vars()





    # BREAKPOINTS ######################################################################################
    def set_break(self, filename, lineno, enabled = True , cond=None ):

        if not  bdb.Bdb.__set_break__( self, filename , lineno, enabled= enabled , cond= cond) :
            err = Json.print_element( "set_break" , "error" , filename=filename, lineno = lineno );
            return False
        #print(f"break setted at {filename}:{lineno}", file = sys.stddbg )
        return True


    def do_set_breaks(self, args  ):
        args = io.StringIO(args)
        for b in json.load( args ):
            #Json.print_element( "set_break","break", value=str(b) )
            self.set_break( b['filename'] , b['lineno'] , enabled = b['enabled']  )


    do_sb = do_set_breaks

    def do_list_breaks(self,*args):

        breaks = []
        for filename in self.breaks:
            bs = self.breaks[filename]
            for lineno in bs:
                b = bs[lineno]

                breaks.append( { 'filename':b.filename , 'lineno':f"{b.lineno}" } )

        d = Json( "breaks")
        d.data['value'] = breaks
        d.print()

    do_lb = do_list_breaks




    # OTHERS ######################################################################################

    def do_cmdline(self,line):

        from io import StringIO


        def displayhook( obj):
            """
               Custom displayhook for the exec in default(), which prevents
               assignment of the _ variable in the builtins.
            """

            # reproduce the behavior of the standard displayhook, not printing None
            if obj is not None:
                print(str(obj))


        stdout = StringIO()

        try:

            code = compile( line + '\n', '<stdin>', 'single')
            save_stdout = sys.stdout
            save_displayhook = sys.displayhook

            if self.frame:
                _locals = self.frame.f_locals
                _globals = self.frame.f_globals
            else:
                _locals = locals()
                _globals = globals()

            try:

                sys.stdout = stdout
                sys.displayhook = displayhook
                exec(code, _globals, _locals)

            finally:
                sys.stdout = save_stdout
                sys.displayhook = save_displayhook

            print( stdout.getvalue() )

        except Exception as e :
            self.print_exception(e)








def main( run_type , debugger_hook, filename , breaks = None ):


    try:


        d = Dbg(debugger_hook)


        filename = d.canonic( filename )


        for (path , lineno, enabled ) in breaks :
            d.set_break( path , lineno , enabled = enabled )


        d.runscript( run_type, filename )

    except Exception as e :

        buf = io.StringIO()
        traceback.print_exception(e, file = buf )
        Json.print_exception( "exception" , "stack" , value = buf.getvalue() )



