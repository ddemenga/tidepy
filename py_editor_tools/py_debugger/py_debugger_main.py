import sys
import os
import io
import json

#import threading
#print(f'DBG:Python started :[{threading.get_native_id()}]')


old_stdout = sys.stdout
old_stderr = sys.stderr
old_stdin = sys.stdin

class DebuggerHook:

    def __init__(self):
        # this is the py_dbg.Cmd class function that must used to send line commands
        # while the program is waiting for input
        self.hook = None

    def do_cmd( self ,line ):
        if self.hook :
            self.hook( line.strip()  )

debugger_hook = DebuggerHook();



class Stdout:

    def write(self, str):

        print( f"OUT:{str.replace(' ',chr(0x00))}"  , file = old_stdout)
        old_stdout.flush()

    def flush(self):
        old_stdout.flush()

    def isatty(self):
        return True

class Stddbg:

    def write(self, str):
        print( f"DBG:{str.replace(' ',chr(0x00))}" , file = old_stdout)
        old_stdout.flush()

    def flush(self):
        old_stdout.flush()

class Stderr:

    def write(self, str):
        print( f"ERR:{str.replace(' ',chr(0x00))}" , file = old_stdout)
        old_stdout.flush()

    def flush(self):
        old_stdout.flush()


old_input = input
def input_debugger(prompt=None):
    if prompt is not None:
        print(f"OUT:{prompt.replace(' ',chr(0x00))}", file = old_stdout)
    print("IND:", file = old_stdout)
    value = old_input()
    while not value.startswith("IND:"):
        print( f"ERR:value is not IND: type {value}" , file = old_stdout)
        print("IND:", file = old_stdout)
        value = old_input()
    return value[4:]


def input(prompt=None):
    if prompt is not None:
        print(f"OUT:{prompt.replace(' ',chr(0x00))}", file = old_stdout)
    print("INU:", file = old_stdout)
    value = old_input()
    while not value.startswith("INU:"):
        debugger_hook.do_cmd(value[4:]) # skip IND:
        #print( f"ERR:value is not INU: type {value}" , file = old_stdout)
        #print( f"test debugger_hook { debugger_hook }" , file = old_stdout)
        print("INU:", file = old_stdout)
        value = old_input()
    return value[4:]



old_exit = exit

def exit( code ):

    print('DBG:EXIT:USER:' , file = old_stdout )
    input_debugger()

    old_exit(code)

__builtins__.input = input
__builtins__.input_debugger = input_debugger
__builtins__.exit = exit





if __name__ == "__main__":


    try:

        sys.__stdout__  = sys.stdout = Stdout()
        sys.stddbg = Stddbg()
        sys.__stderr__ = sys.stderr = Stderr()
        #sys.__stdin__ = sys.stdin = Stdin()


        # == GET SETINGS =======
        init_data = input_debugger()
        args = json.load( io.StringIO(init_data) )
        #print( f"data={args}",file=sys.stddbg )

        # == ARGV ========
        sys.argv = [ args["script"] ]
        for a in args['args']:
            sys.argv.append( a )

        # == PATH ========
        if os.path.dirname(__file__) not in sys.path:
            sys.path.insert( 0, os.path.dirname(__file__) )

        for p in reversed(args['path']):
            if p not in sys.path:
                sys.path.insert(0,p)

        # == ENV =========
        for k,v in args['env'].items():
            os.environ[k] = v




        #print('Breakpoints:')
        breaks = []
        for b in args['breaks']:
            breaks.append( ( b["filename"],b["lineno"],b["enabled"]) )
            #print(f'break : filename={b["filename"]} , lineno = {b["lineno"]} ')

        #print(sys.path)

        import py_dbg

        # change dir to project root directory
        os.chdir(args['wdir'])

        if args['run_type'] == "run":
            print(f'run script: {args["script"]}', file = sys.stddbg )
        else:
            print(f'debugging script: {args["script"]}', file = sys.stddbg )
        print(f'interpreter : {args["python"]}', file = sys.stddbg )
        print(f'working dir = {os.getcwd()}', file = sys.stddbg )
        print(f'sys.argv = {sys.argv}', file = sys.stddbg )

        py_dbg.main( args['run_type'],debugger_hook, args['script'] , breaks )


    except Exception as e :
        import traceback
        traceback.print_exception(e)

    finally:

        print('DBG:EXIT:' , file = old_stdout )
        print("IND:", file = old_stdout)
        old_input()





