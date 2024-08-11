import ast


class PlainTextPrinter:

    def __init__(self, stdout ):
        self.out = stdout
        self.level = 0

    def open_tag(self, node , type_tag , id ):
        print( f"{''.join([' ']*self.level) }{node.lineno} {node.col_offset} {type_tag} {id}" ,file=self.out )
        self.level +=2

    def open_close_tag(self, node , type_tag , id ):
        print( f"{''.join([' ']*self.level) }{node.lineno} {node.col_offset} {type_tag} {id}" ,file=self.out )

    def close_tag(self, type_tag ):
        self.level -=2
        pass


class XmlPrinter :

    def __init__(self, stdout ):
        self.out = stdout
        self.level = 0
        #print('<?xml version="1.0" encoding="UTF-8" standalone="yes" ?>')

    def open_tag(self, node , type_tag , id ):
        print ( f"{''.join([' ']*self.level) }<{type_tag} line='{node.lineno}' column='{node.col_offset}' id='{id}'>" ,file=self.out )
        self.level +=2

    def open_close_tag(self, node , type_tag , id ):
        print ( f"{''.join([' ']*self.level) }<{type_tag} line='{node.lineno}' column='{node.col_offset}' id='{id}'></{type_tag}>",file=self.out  )

    def close_tag(self, type_tag ):
        self.level -=2
        print (  f"{''.join([' ']*self.level) }</{type_tag}>", file=self.out )



class Browser(ast.NodeVisitor):

    def __init__(self, tree, stdout ):

        self.stack = []
        self.attribute_filter =  None
        self.printer = XmlPrinter(stdout)

        self.visit(tree)

    def __styled_msg__(self,  node, msg ):
        return f"{''.join([' ']*self.level) }{node.lineno}:{node.col_offset} {msg}"

    def visit_ClassDef(self, node):

        #self.trace.append( self.__styled_msg__( node, f"class {node.name}") )

        self.attribute_filter = []

        self.stack.append(node)

        self.printer.open_tag( node, "class", node.name )

        self.generic_visit(node)
        self.stack.pop()

        self.printer.close_tag( "class")

    def visit_FunctionDef(self, node):


        if len(self.stack) and type(self.stack[-1]) == ast.ClassDef:
            self.printer.open_tag( node, "member_function", node.name )
            #self.trace.append( self.__styled_msg__( node, f"class function {self.stack[-1].name} {node.name}") )

        else:
            self.printer.open_tag( node, "function", node.name )
            #self.trace.append( self.__styled_msg__( node, f"function  {node.name}") )


        self.stack.append(node)


        self.generic_visit(node)


        self.stack.pop()
        self.printer.close_tag( "function")

    def visit_Assign(self, node ):


        for target in node.targets :

            if type( target ) == ast.Name and len(self.stack) and type(self.stack[-1]) == ast.ClassDef :
                self.printer.open_close_tag( target, "member_var", target.id )
                #self.trace.append( self.__styled_msg__( node,  f"class varname {target.id} ") )

            elif type( target ) == ast.Name and not(self.stack):
                self.printer.open_close_tag(  target, "var", target.id )
                #self.trace.append( self.__styled_msg__( node,  f"var {target.id} ") )

        self.stack.append(node)
        self.generic_visit(node)
        self.stack.pop()



    def visit_Attribute(self, node ):

        if type( node.value ) == ast.Name and node.value.id == 'self' and type(node.value.ctx) == ast.Load:

            if not node.attr in self.attribute_filter:

                self.printer.open_close_tag( node, "member_attr", node.attr )
                self.attribute_filter.append( node.attr )
                #self.trace.append( self.__styled_msg__( node, f"class attr self.{node.attr} ") )




    def visit_Import(self, node):
        for name in node.names:
            if name.asname is not None:
                self.printer.open_close_tag( node, "import", f"{name.name} as {name.asname}" )
                #self.trace.append( self.__styled_msg__( node,  f"import  { name.name +' as '+name.asname }"))
            else:
                self.printer.open_close_tag( node, "import", f"{name.name}" )
                #self.trace.append( self.__styled_msg__( node, f"import  { name.name}") )

    def visit_ImportFrom(self, node):
        for name in node.names:
            if name.asname is not None:
                self.printer.open_close_tag( node, "import_from", f"{name.name} as {name.asname}" )
                #self.trace.append( self.__styled_msg__( node, f"import from  {node.module} { name.name +'['+name.asname+']'}") )
            else:
                self.printer.open_close_tag( node, "import", f"{name.name}" )
                #self.trace.append(self.__styled_msg__( node, f"import from  {node.module} { name.name}"))

    def visit_Global(self,node):
        for name in node.names:
            self.printer.open_close_tag( node, "global", name )
        #print( self.__styled_msg__( node, f"global  {node.name}"))
    visit_Nonlocal = visit_Global

def parse_text( text ):
    import sys
    import os
    import io

    try:
        #/home/userd/3rdparty/eclipse/tmp/ml_studies/nnetlib.py
        tree = ast.parse( text , "tmp.py", 'exec')

        s = io.StringIO()
        Browser( tree , s )

        s.seek(0)
        return "".join( s.readlines() )

    except Exception as e:
        return ""


if __name__ == "__main__":

    import sys
    
    if len(sys.argv) >= 2:   
        filepath = sys.argv[1]
                
        print( parse_text( "".join(open(filepath).readlines()) ) )

    else:
        print("<error_missing_filepath>")    

    







