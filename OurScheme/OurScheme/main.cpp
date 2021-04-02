# include <iostream>
# include <string>
# include <stdlib.h>
# include <vector>
# include <stack>
# include <exception>
# include <iomanip>

using namespace std;

// If only one char, then define as the multiple of 11
// If is a boolean, then define as the multiple of 13
// LPAREN 11*97  '('
// RPAREN 22*97  ')'
// INT 12*97  e.g., '123', '+123', '-123'
// STRING 14*97  doesn't cross line, inclue following char >\n<, >\"<, >\t<, >\\<
// DOT 33*97 '.'
// FLOAT 16*97 '123.567', '123.', '.567', '+123.4' , '-.123'
// NIL 13*97 'nil', '#f' only these two possible look
// T 26*97 't', '#t' only these two possible look, too
// QUOTE 44*97 '
// SYMBOL 18*97  // DO NOT contain '(', ')', '\'', '\"', white-space

enum TokenType { LPAREN = 1067, RPAREN = 2134, INT = 1164, STRING = 1358, DOT = 3201, FLOAT = 1552, NIL = 1261, T = 2522, QUOTE = 4268, SYMBOL = 1746 } ;

enum NodeType { EMPTY = 0, ATOM = 1, CONS = 2, SPECIAL = 3 } ;

static int gTestNum = 0 ;  // test num from PAL
static int gLine = 1 ;  // the line of the token we recently "GET"
static int gColumn = 0 ; // // the golumn of the token we recently "GET"
static string gPeekToken = "" ;  // the recent token we peek BUT haven't "GET"
static bool gIsEOF = false ; // if is TRUE means there doesn't have '(exit)'

struct Token {
    string str ;  // the original apperance read from input
    int line ;  // the line which this token exist
    int column ;  // the column where this token exist
    TokenType type ;  // type of the token

    Token() {
        str = "" ;
        line = 0 ;
        column = 0 ;

    } // constructor
} ;

string enumToStr( TokenType type ) {
    switch ( type ) {
        case LPAREN:
            return "LPAREN" ;
            break ;
        case RPAREN :
            return "RPAREN" ;
            break ;
        case INT :
            return "INT" ;
            break ;
        case STRING :
            return "STRING" ;
            break ;
        case DOT :
            return "DOT" ;
            break ;
        case FLOAT :
            return "FLOAT" ;
            break ;
        case NIL :
            return "NIL" ;
            break ;
        case T :
            return "T" ;
            break ;
        case QUOTE :
            return "QUOTE" ;
            break ;
        case SYMBOL :
            return "SYMBOL" ;
            break ;
            
        default:
            break;
    }
} // enumToStr()

struct Node_Linear {
    Token token ;
    Node_Linear* next ;
    Node_Linear* prev ;
    
    Node_Linear(): next( NULL ), prev( NULL ) {} ;
} ;

class GlobalFunction { // the functions that may be used in anywhere
    
public:
    string intToStr( int num ) {
        string str = "" ;
        if ( num == 0 ) return "0" ;
        
        while ( num != 0 ) {
            str = ( char ) ( '0' + ( num % 10 ) ) + str ;
            num /= 10 ;
        } // while()
        
        return str ;
    } // intToStr()
    
    int getValueOfIntStr( string str ) {
        int num = 0 ;
        char sign = '\0' ;
        
        if ( str[ 0 ] == '+' || str[ 0 ] == '-' ) {
            sign = str[ 0 ] ;
            str.erase( str.begin(), str.begin() + 1 ) ; // take off the sign char
        } // if()
        
        num = atoi( str.c_str() ) ;
        
        if ( sign == '-' ) {
            num *= -1 ;
        } // if()
        
        return num ;
    } // getValueOfIntStr()
    
    float getValueOfFloatStr( string str ) {
        float num = 0.0 ;
        char sign = '\0' ;
        
        if ( str[ 0 ] == '+' || str[ 0 ] == '-' ) {
            sign = str[ 0 ] ;
            str.erase( str.begin(), str.begin() + 1 ) ; // take off the sign char
        } // if()
        
        num = atof( str.c_str() ) ;
        
        if ( sign == '-' ) {
            num *= -1.0 ;
        } // if()
        
        return num ;
    } // getValueOfFloatStr
    
} ;

GlobalFunction g ;

class MissingAtomOrLeftParException : public exception {
private:
    int line ;
    int col ;
    string str ;
    
public:
    MissingAtomOrLeftParException( int l, int c, string s ) : line( l ), col( c ), str( s ) {}
    string err_mesg() {
        string mesg = "" ;
        mesg = "ERROR (unexpected token) : atom or '(' expected when token at Line " + g.intToStr( line ) + " Column " + g.intToStr( col ) + " is >>" + str + "<<" ;
        
        return mesg ;
    } // err_mesg()
} ; // MissingAtomOrLeftParException

class MissingRightParException : public exception {
private:
    int line ;
    int col ;
    string str ;
    
public:
    MissingRightParException( int l, int c, string s ) : line( l ), col( c ), str( s ) {}
    string err_mesg() {
        string mesg = "" ;
        mesg = "ERROR (unexpected token) : ')' expected when token at Line " + g.intToStr( line ) + " Column " + g.intToStr( col ) + " is >>" + str + "<<" ;
        
        return mesg ;
    } // err_mesg()
} ; // MissingRightParException

class NoClosingQuoteException : public exception {
private:
    int line ;
    int col ;
    
public:
    NoClosingQuoteException( int l, int c ) : line( l ), col( c ) {}
    string err_mesg() {
        string mesg = "" ;
        mesg = "ERROR (no closing quote) : END-OF-LINE encountered at Line " + g.intToStr( line ) + " Column " + g.intToStr( col ) ;
        
        return mesg ;
    } // err_mesg()
} ; // NoClosingQuoteException

class EOFException : public exception {
public:
    string err_mesg() {
        string mesg = "" ;
        mesg = "ERROR (no more input) : END-OF-FILE encountered" ;
        
        return mesg ;
    } // err_mesg()
} ; // EOFException

class SingleList {
    
public:
    
    Node_Linear* root ;
    Node_Linear* tail ;
    
    Node_Linear* findNode( Token token ) {
        Node_Linear* nodeWeWant = NULL ;
        
        for ( Node_Linear* walk = root ; walk != NULL && nodeWeWant != NULL ; walk = walk -> next ) {
            if ( token.str == walk -> token.str && token.line == walk -> token.line && token.column == walk -> token.column ) {
                nodeWeWant = walk ;
            } // if()
        } // for()
        
        return nodeWeWant ;
    } // findNode()
    
    // Purpose: Simply add a new node at the tail
    void addNode( Token token ) {
        Node_Linear* newNode = new Node_Linear ;
        newNode -> token = token ;
        
        if ( root == NULL ) { // empty
            root = newNode ;
            newNode -> prev = root ;
            tail = newNode ;
        } // if()
        else {
            bool addSuccess = false ;
            
            for ( Node_Linear* walk = root ; walk != NULL && !addSuccess ; walk = walk -> next ) {
                if ( walk -> next == NULL ) {
                    walk -> next = newNode ;
                    newNode -> prev = walk ;
                    tail = newNode ;
                    addSuccess = true ;
                } // if()
            } // for()
        } // else()
        
    } // addNode()
    
    // Purpose: used to make up some DOT and () and NIL
    void insertNode( Node_Linear* nodeBefore, TokenType type ) {
        Token token ;
        token.line = -1 ;
        token.column = -1 ;
        token.type = type ;
        if ( type == DOT ) {
            token.str = "." ;
        } // if()
        else if ( type == LPAREN ) {
            token.str = "(" ;
        } // else if()
        else if ( type == RPAREN ) {
            token.str = ")" ;
        } // else if()
        else if ( type == NIL ) {
            token.str = "nil" ;
        } // else if()
        
        // assert: all information for this token has done
        
        Node_Linear* newNode = new Node_Linear ;
        newNode -> token = token ;
        if ( nodeBefore == root ) {
            newNode -> next = root ;
            newNode -> prev = root ;
            newNode -> next -> prev = newNode ;
            root = newNode ;
        } // if()
        else {
            newNode -> next = nodeBefore -> next ;
            newNode -> prev = nodeBefore ;
            newNode -> next -> prev = newNode ;
            nodeBefore -> next = newNode ;
        } // else()
        
    } // insertNode()
    
    void print() {
        for ( Node_Linear* walk = root ; walk != NULL ; walk = walk -> next ) {
            cout << walk -> token.str << "  (" << walk -> token.line << ", " << walk -> token.column << " ) " << enumToStr( walk -> token.type ) << endl ;
        } // for()
        cout << endl ;
    } // print()
    
    void printForward() {
        cout << endl << "*** Print forward ***" << endl ;
        for ( Node_Linear* walk = root ; walk != NULL ; walk = walk -> next ) {
            cout << walk -> token.str << " " ;
        } // for()
    } // printForward()
    
    void printBackforward() {
        bool finish = false ;
        cout << endl << "*** Print backward ***" << endl ;
        for ( Node_Linear* walk = tail ; !finish ; walk = walk -> prev ) {
            cout << walk -> token.str << " " ;
            if ( walk == root ) finish = true ;
        } // for()
    } // printBackforward()
    
    void clear() {
        while ( root != NULL ) {
            Node_Linear* current = root ;
            root = root -> next ;
            delete current ;
            current = NULL ;
        } // while()
    } // clear()
    
} ;

SingleList singleList ;

class LexicalAnalyzer {
    
private:

    bool isSeparator( char ch ) {
        if ( ch == '(' || ch == ')' || ch == '\'' || ch == '\"' || ch == ';' ) {
            return true ;
        } // if()

        return false ;
    } // isSepatator()
    
    bool isReturnLine( char ch ) {
        if ( ch == '\n' || ch == '\r' ) {  // Because Linux has '\r' character, I added '\r' as one circumstance
            return true ;
        } // if()
            
        return false ;
    } // isReturnLine()
    
    bool isWhiteSpace( char ch ) {
        if ( ch == ' '|| ch == '\t' || isReturnLine( ch )) {  // Because Linux has '\r' character, I added '\r' as one circumstance
            return true ;
        } // if()

        return false ;
    } // isWhiteSpace()
    
    string formatFloat( string str ) {
        string formatStr = "" ;
        
        if ( str[ str.length() - 1 ] == '.' ) { // float num end with a dot
            formatStr = str + "000" ; // put some zero in it
        } // if()
        else if ( str[ 0 ] == '.' ) { // float num start with the dot
            formatStr = "0" + str ;
        } // else if()
        else {
            int dotIndex = ( int ) str.find( '.' ) ;
            int count = ( int )str.length() - dotIndex ;
            formatStr = str ;
            for ( int i = 0 ; i < count ; i ++ ) {
                formatStr = formatStr + "0" ;
            } //  for()
            
        } // else()
        
        return formatStr ;
    } // formatFloat()
        
    // Purpose: not only call the func. cin.get(), but also increase the column or line
    char getChar() {
        char ch = '\0' ;
        ch = cin.get() ;
        if ( isReturnLine( ch ) ) {
            gLine ++ ;
            gColumn = 0 ;
        } // if()
        else  {
            gColumn ++ ;
        } // else()
        
        return ch ;
    } // getChar()
    
    string getFullStr( string fullStr ) {
        // assert: 'ch' must be a peeked char which is '\"'
        bool keepRead = true ;
        char ch_get = '\0' ;
        char ch_peek = '\0' ;
            
        ch_peek = cin.peek() ;
        // because we need to get a string, keep reading the input until encounter the next '\"' or return-line
        while ( keepRead && !isReturnLine( ch_peek ) ) {
            ch_get = getChar() ;
            fullStr += ch_get ;
            ch_peek = cin.peek() ;
            
            if ( ch_peek == '\"' && ch_get != '\\' )  { // >"< stands alone
                keepRead = false ;
            } // if()
        } // while()
            
        if ( ch_peek == '\"' ) {  // a complete string with a correct syntax
            ch_get = getChar() ;
            fullStr += ch_get ;
        } // if()
        
        return fullStr ;
    } // getFullStr()
    
    Token lexToToken( string lex ) {
        Token token ;
        if ( lex != "." && isFLOAT( lex ) ) {
            // assert: float with a original format
            // now  can start trandfer the float into the format which (int).(3 chars)
            token.str = formatFloat( lex ) ;
        } // if()
        else {
            token.str = lex ;
        } // else()
        token.line = gLine ;
        token.column = gColumn - ( int ) lex.length() + 1 ;
        token.type = getTokenType( lex ) ;
        
        return token ;
    } // setTokenInfo()

public:
    
    bool isINT( string str ) {
        // Mark1: there might be a sign char, such as '+' or '-'
        // Mark2: except the sign char, other char should be a number
        // Mark3: the  whole string cannot contain the dot
        int startIndex = 0 ;  // to avoid the sign char if there has one
            
        if ( str[ 0 ] == '+' || str[ 0 ] == '-' ) {
            startIndex = 1 ;  // the checking process start after the sign char
        } // if()
            
        for ( int i = startIndex ;  i < str.length() ; i ++ ) {
            if ( !( str[ i ] <= '9' && str[ i ] >= '0' ) ) {
                return false ;
            } // if()
        } // for()
            
        return true ;
    } // isINT()
    
    // Purpose: recognize whether this string is a FLOAT
    // Return: true or false
    bool isFLOAT( string &str ) {
        // Mark1: there might be a sign char, such as '+' or '-'
        // Mark2: except the sign char, other char should be a number
        // Mark3: the whole string SHOULD contain the dot, NO MATTER the position of the dot is, but should be only dot
        // Mark4: if there appear another dot after already get one, then might be a SYMBOL
        int dotNum = 0 ;  // only can have ONE dot
        int startIndex = 0 ;  // the checking process start after the sign char
        
        if ( str[ 0 ] == '+' || str[ 0 ] == '-' ) {
            startIndex = 1 ;  // the checking process start after the sign char
        } // if()
            
        for ( int i = startIndex ;  i < str.length() ; i ++ ) {
            if ( str[ i ] == '.' ) {
                dotNum ++ ;  // every time we encounter a dot, count it
            } // if()
                
            if ( !( str[ i ] <= '9' && str[ i ] >= '0' ) && str[ i ] != '.' ) {
                return false ;
            } // if()
        } // for()
            
        if ( dotNum != 1 ) {
            return false ;
        } // if()
        
        return true ;
    } // isFLOAT()
    
    // Purpose: accept the token string from func. getToken(), and response the corresponding token value
    TokenType getTokenType( string str ) {
        
        if ( str == "(" ) {  // Left parameter
            return LPAREN ;
        } // if()
        else if ( str == ")" ) {  // Right parameter
            return RPAREN ;
        } // else if()
        else if ( str == "." ) {  // Dot
            return DOT ;
        } // else if()
        else if ( str == "\'" ) {  // Quote
            return QUOTE ;
        } // else if()
        else if ( str == "nil" || str == "#f" ) {  // NIL
            return NIL ;
        } // else if()
        else if ( str == "t" || str == "#t" ) {  // T
            return T ;
        } // else if()
        else if ( isINT( str ) ) {
            return INT ;
        } // else if()
        else if ( isFLOAT( str ) ) {
            return FLOAT ;
        } // else if()
        else if ( str[ 0 ] == '\"' ) {
            return STRING ;
        } // else if()
        
        return SYMBOL ;  // none of the above, then assume it's symbol
    } // getTokenType()
    
    string peekToken() {
        string tokenStrWeGet = "" ;
        
        if ( gPeekToken == "" ) {
            char ch = '\0' ;

            ch = cin.peek() ;  // peek whether the next char is in input
            if ( ch == EOF ) {
                throw EOFException() ;
            } // if()
            
            while ( isWhiteSpace( ch ) ) {  // before get a actual char, we need to skip all the white-spaces first
                ch = cin.get() ;  // take away this white-space
                gColumn ++ ;
                
                if ( isReturnLine( ch ) ) {
                    gLine ++ ;
                    gColumn = 0 ;
                } // if()
                
                ch = cin.peek() ;  // keep peeking next char
            } // while()

            // assert: finally get a char which is not a white-space, now can start to construct a token
            ch = getChar() ;  // since this char is not a white-space, we can get it from the input
            tokenStrWeGet += ch ;  // directly add the first non-white-space char into the token string

            // if this char is already a separator then STOP reading, or keep getting the next char
            if ( !isSeparator( ch ) && ch != '\"' ) {  // 'ch' here is the first char overall
                ch = cin.peek() ;

                while ( !isSeparator( ch ) && !isWhiteSpace( ch ) ) {
                    ch = getChar() ;
                    tokenStrWeGet += ch ;
                    ch = cin.peek() ;
                } // while()
                
            } // if()
            else if ( ch == '\"' ) {
                // assert: we get the whole token
                tokenStrWeGet = getFullStr( tokenStrWeGet ) ;
            } // else if()

            // assert: we get the whole token
            gPeekToken = tokenStrWeGet ;
        } // if()
        
        return gPeekToken ;
        
    } // peekToken()

    Token getToken() {
        if ( gPeekToken == "" ) peekToken() ;
        
        if ( gPeekToken == ";" ) { // encounter a line comment
            char ch = getchar() ;
            while ( !isReturnLine( ch ) ) {
                ch = getchar() ;
            } // while()
            gLine ++ ;
            gColumn = 0 ;
            gPeekToken = "" ;
            peekToken() ;
        } // if()
        
        Token tokenWeWant = lexToToken( gPeekToken ) ;
        gPeekToken = "" ;
        singleList.addNode( tokenWeWant ) ;

        return tokenWeWant ;
    } // getToken()

};

// Purpose: Check the statement, if nothin wrong the build the tree, else print the error
class SyntaxAnalyzer {
    
private:
    
    LexicalAnalyzer la ;
    
public:
    
    bool checkSExp( Token startToken ) {
        // assert: startToken can only has three possibility
        // 1.Atom 2.LP 3.Quote *4.LR RP
        
        if ( startToken.type == LPAREN && la.getTokenType( la.peekToken() ) == RPAREN ) { // this is a NIL with special format >()<
            la.getToken() ; // take away the RP from the input
            
            return true ; // one of an ATOM
        } // if()
        else if ( isATOM( startToken ) ) {
            return true ;
        } // else if()
        else if ( startToken.type == QUOTE ) {
            Token token = la.getToken() ; // get the next token, suppose to be the start of a S-exp
            
            return  checkSExp( token ) ;
        } // else if()
        else if ( startToken.type == LPAREN ) {
            // suppose to have at least ONE S-exp
            bool hasOneSExpCorrect = false ;
            bool moreSExpCorrect = true ;
            
            Token token = la.getToken() ; // get the next token, suppose to be the start of a S-exp
            hasOneSExpCorrect = checkSExp( token ) ;
            
            if ( hasOneSExpCorrect ) {
                
                while ( ( la.getTokenType( la.peekToken() ) == LPAREN || isATOM( la.peekToken() ) || la.getTokenType( la.peekToken() ) == QUOTE ) && moreSExpCorrect ) {
                    token = la.getToken() ;
                    moreSExpCorrect = checkSExp( token ) ;
                    
                    la.peekToken() ; // maybe successfully check a correct S-exp, keep peeking the next one
                } // if()
                
                if ( !moreSExpCorrect ) { // there are more S-exp, but not all correct
                    return false ;
                } // if()
                else {
                    if ( la.getTokenType( la.peekToken() ) == DOT ) { // means only one S-exp in this left S-exp
                        token = la.getToken() ; // must be DOT
                        token = la.getToken() ; // must be the start of the next S-exp according to the grammer
                        
                        hasOneSExpCorrect = checkSExp( token ) ;
                        if ( !hasOneSExpCorrect ) {
                            throw MissingAtomOrLeftParException( gLine, gColumn, gPeekToken ) ;
                            return false ;
                        } // if()
                        else {
                            if ( la.getTokenType( la.peekToken() ) == RPAREN ) {
                                token = la.getToken() ; // must be >)<
                                
                                return true ;
                            } // if()
                            else {
                                throw MissingRightParException( gLine, gColumn, gPeekToken ) ;
                                return false ;
                            } // else()
                        } // else()
                        
                    } // if()
                    else if ( la.getTokenType( la.peekToken() ) == RPAREN ) {
                        token = la.getToken() ; // must be >)<
                        
                        return true ;
                    } // else if()
                } // else()
                
            } // if()
            else {
                throw MissingAtomOrLeftParException( gLine, gColumn, token.str ) ;
            } // else()
            
            return false ;
        } // else if()
        
        // throw MissingAtomOrLeftParException( gLine, gColumn, gPeekToken ) ;
        
        return false ; // none of the above begining
        
    } // checkSExp()

    bool isATOM( Token token ) {
        TokenType type = token.type ;
        if ( type == SYMBOL || type == INT || type == FLOAT || type == STRING || type == NIL || type == T ) {
            return true ;
        } // if()
        
        return false ;
    } // checkATOM()
    
    bool isATOM( string str ) {
        TokenType type = la.getTokenType( str ) ;
        if ( type == SYMBOL || type == INT || type == FLOAT || type == STRING || type == NIL || type == T ) {
            return true ;
        } // if()
        
        return false ;
    } // isATOM()

} ;

struct Node {
    string lex ; // the string (what it looks in the input file) of this token
    NodeType type ; // Three possibility: 1.Atom  2.Special(NIL)  3.Cons
    Node* left ;
    Node* right ;
    Node* parent ;

    Node() : lex(""), type(EMPTY), left(NULL), right(NULL), parent(NULL) {} ;
};

class Tree {
    
private:
    
    Node *root ;
    SingleList copyList ;
    
    LexicalAnalyzer la ;
    SyntaxAnalyzer s ;
    
    enum Direction { RIGHT = 1234, LEFT = 4321 } ;
    
    void transferNIL( Node_Linear* root, Node_Linear* tail ) {
        bool finish = false ;
        
        if ( root -> token.type == LPAREN && root -> next -> token.type == RPAREN ) {
            Node_Linear* nilNode = new Node_Linear ;
            nilNode -> token.str = "nil" ;
            nilNode -> token.type = NIL ;
            nilNode -> token.line = root -> token.line ;
            nilNode -> token.column = root -> token.column ;
            
            nilNode -> next = root -> next -> next ;
            
            // Delete >)<
            delete root -> next ;
            root -> next = NULL ;
            // Delete >(<
            delete root ;
            root = NULL ;
            
            root = nilNode ;
        } // if()
        
        for ( Node_Linear* walk = root ; walk -> next != NULL && walk -> next -> next != NULL && !finish ; walk = walk -> next ) {
            
            if ( walk -> next -> token.type == LPAREN && walk -> next -> next -> token.type == RPAREN ) {
                Node_Linear* nilNode = new Node_Linear ;
                nilNode -> token.str = "nil" ;
                nilNode -> token.type = NIL ;
                nilNode -> token.line = walk -> next -> token.line ;
                nilNode -> token.column = walk -> next -> token.column ;
                
                nilNode -> next = walk -> next -> next -> next ;
                walk -> next -> next -> next -> prev = nilNode ;
                
                // Delete >)<
                delete walk -> next -> next ;
                walk -> next -> next = NULL ;
                // Delete >(<
                delete walk -> next ;
                walk -> next = nilNode ;
                nilNode -> prev = walk ;
            } // if()
            
        } // for()
        
    } // transferNIL()
    
    Node_Linear* findStrAndGetPreviousNode( Node_Linear* root, Node_Linear* tail, string str ) {
        
        Node_Linear* target = NULL ;
        
        if ( str == root -> token.str ) {
            target = root ;
            return target ;
        } // if()
        
        for ( Node_Linear* walk = root ; walk != tail && target == NULL ; walk = walk -> next ) {
            if ( str == walk -> token.str ) {
                target = walk -> prev ;
            } // if()
        } // for()
        
        return target ;
        
    } // findStrAndGetPreviousNode()
    
    Node_Linear* findCorrespondPar( Node_Linear* par_L ) {
        stack<Node_Linear*> nodeStack ;
        Node_Linear* target = NULL ; // the pointer that pointed to the Right parathesis
        
        nodeStack.push( par_L ) ; // put the first left parathesis in the stack
        for ( Node_Linear* walk = par_L -> next ; target == NULL && !nodeStack.empty() ; walk = walk -> next ) {
            if ( walk -> token.type == RPAREN ) {
                // when encounter a right par, then keep pop out the items util meet the first left par
                Node_Linear* node_pop = nodeStack.top() ;
                while ( node_pop -> token.type != LPAREN ) {
                    nodeStack.pop() ;
                    node_pop = nodeStack.top() ;
                } // while()
                nodeStack.pop() ; // pop out the left par
                
                if ( nodeStack.empty() ) { // this left par is the last one
                    target = walk ;
                } // if()
            } // if()
            else {
                nodeStack.push( walk ) ;
            } // else()
        } // for()
        
        return target ;
    } // findCorrespondPar()
    
    Node_Linear* findDOT( Node_Linear* root, Node_Linear* tail ) {
        stack<Node_Linear*> s ;
        int count = 0 ;
        
        s.push( tail ) ;
        if ( tail -> prev -> token.type != RPAREN ) {
            if (  tail -> prev -> prev -> token.type != DOT ) {
                return NULL ;
            } // if()
            else {
                return tail -> prev -> prev ;
            } // else()
        } // if()
        
        Node_Linear* walk ;
        for ( walk = tail -> prev ; walk != root ; ) {
            if ( walk -> token.type == LPAREN ) {
                while (  s.top() -> token.type != RPAREN ) {
                    if ( s.top() -> token.type == DOT ) count -- ;
                    s.pop() ;
                } // while()
                s.pop() ; // last right par
            } // if()
            else {
                if ( walk -> token.type == DOT ) count ++ ;
                s.push( walk ) ;
            } // else()
            
            if ( walk != root ) walk = walk -> prev ;
            
        } // for()
        
        if ( count == 1 ) {
            while ( s.top() -> token.type != DOT ) { // right part of the cons is a list
                s.pop() ;
            } // if()
            
            return s.top() ;
        } // if()
        
        return NULL ;
        
    } // findDOT()
    
    // Purpose: focus on one S-exp and give it the parathesis
    // Only list can call this function
    void translate( Node_Linear* root, Node_Linear* tail ) {
        
        transferNIL( root, tail ) ; // put a NIL in this list if needed
        
        int countPar = 0 ; // increase when manually add DOT and Paranthesis
        
        Node_Linear* dotPointer = findDOT( root, tail ) ;
        if ( dotPointer == NULL ) { // there is no DOT, so put it on manually
        // if ( findStrAndGetPreviousNode( root, tail, "." ) == NULL || !findDOT( root, tail ) ) { // there is no DOT, so put it on manually
            // assert: there is no DOT so need to add DOT and nil
            copyList.insertNode( tail -> prev, DOT ) ;
            copyList.insertNode( tail -> prev, NIL ) ;
        } // if()
        
        // assert: there must be at least one dot in this S-exp
        bool hasFinish = false ;
        for ( Node_Linear* walk = root -> next ; !hasFinish && walk != tail ; ) {
            if ( walk -> token.type == LPAREN ) {
                Node_Linear* corRightPar = findCorrespondPar( walk ) ;
                translate( walk, corRightPar ) ;
                walk = corRightPar ;
            } // if()
            
            if ( walk -> next -> token.type != DOT ) { // manually add DOT and keep counting
                copyList.insertNode( walk, DOT ) ;
                walk = walk -> next ;
                copyList.insertNode( walk, LPAREN ) ;
                walk = walk -> next ;
                countPar ++ ;
            } // if()
            else {
                walk = walk -> next ; // skip the atom before the DOT
                if ( walk -> next -> token.type == LPAREN ) {
                    Node_Linear* corRightPar = findCorrespondPar( walk -> next ) ;
                    translate( walk -> next, corRightPar ) ;
                    walk = corRightPar ;
                    
                    for ( int i = 0 ; i < countPar ; i ++ ) {
                        copyList.insertNode( walk, RPAREN ) ;
                    } // for()
                } // if()
                else { // only an atom
                    walk = walk -> next ; // skip DOT
                    for ( int i = 0 ; i < countPar ; i ++ ) {
                        copyList.insertNode( walk, RPAREN ) ;
                    } // for()
                } // else()
                
                hasFinish = true ;
            } // else()
            
            if ( walk != NULL ) walk = walk -> next ;
        } // for()
        
    } // translate
    
    void printWhite( int num ) {
        for ( int i = 0 ; i < num ; i ++ ) {
            cout << " " ;
        } // for()
    } // printWhite()
    
public:
    Tree( SingleList list ) : root(NULL), copyList( list ) {} ;
    
    // Purpose: Transfer the DS from list to pointer (tree)
    // Pre-request: tokens in vector construct a S-exp with correct grammer
    // Return the root of this tree
    Node* build( Node_Linear* leftPointer, Node_Linear* rightPointer ) {
        
        if ( leftPointer -> token.type == LPAREN && rightPointer -> token.type == DOT ) { // left part if the cons
            Node* atomNode = new Node ;
            atomNode -> lex = leftPointer -> next -> token.str ;
            if ( leftPointer -> next -> token.type == NIL || leftPointer -> next -> token.type == T ) {
                atomNode -> type = SPECIAL ;
            } // if()
            else {
                atomNode -> type = ATOM ;
            } // else()
            atomNode -> left = NULL ;
            atomNode -> right = NULL ;
            
            return atomNode ;
        } // if()
        else if ( leftPointer == rightPointer ) { // right part of the cons
            Node* atomNode = new Node ;
            atomNode -> lex = leftPointer -> token.str ;
            if ( leftPointer -> token.type == NIL || leftPointer -> token.type == T ) {
                atomNode -> type = SPECIAL ;
            } // if()
            else {
                atomNode -> type = ATOM ;
            } // else()
            atomNode -> left = NULL ;
            atomNode -> right = NULL ;
            
            return atomNode ;
        } // else if()
        else { // Now is still in ( ) form
            Node_Linear* dotPointer = findDOT( leftPointer, rightPointer ) ;
            Node* leftSubTree = build( leftPointer -> next, dotPointer -> prev ) ;
            Node* rightSubTree = build( dotPointer -> next, rightPointer -> prev ) ;
            
            Node* cons = new Node ;
            cons -> type = CONS ;
            cons -> left = leftSubTree ;
            cons -> left -> parent = cons ;
            cons -> right = rightSubTree ;
            cons -> right -> parent = cons ;
            
            return cons ;
        } // else()
        
        return NULL ;
    } // build()

    void prettyPrintAtom( Node* r ) {
        if ( r -> type == SPECIAL ) {
            if ( r -> lex == "nil" || r -> lex == "#f" ) {
                cout << "nil" << endl ;
            } // if()
            else {
                cout << "#t" << endl ;
            } // else()
        } // if()
        else if ( la.isINT( r -> lex ) ) {
            cout << g.getValueOfIntStr( r -> lex ) << endl ;
        } // else()
        else if ( la.isFLOAT( r -> lex ) ) {
            cout << fixed << setprecision( 3 ) << g.getValueOfFloatStr( r -> lex ) << endl ;
        } // else if()
        else cout << r -> lex << endl ;
    } // prettyPrintAtom()
    
    void prettyPrintSExp( Node* r, Direction dir, int level ) {
        
        int curLevel = level ;
        
        if ( r -> type == ATOM || r -> type == SPECIAL ) {
            // case1. LL case2. RL case3. RR
            if ( dir == LEFT ) {
                printWhite( curLevel + 2 ) ;
                prettyPrintAtom( r -> left ) ;
            } // if()
            else {
                if ( r -> lex != "nil" && r -> lex != "#f" ) {
                    printWhite( curLevel + 2 ) ;
                    cout << "." << endl ;
                    printWhite( curLevel + 2 ) ;
                    prettyPrintAtom( r ) ;
                } // if()
                
                printWhite( curLevel ) ;
                cout << ")" << endl ;
            } // else()
            
            return ;
        } // if()
        else { // CONS node
            
            if ( dir == LEFT ) {
                if ( r -> left -> type != CONS ) {
                    cout << "(" << " " ;
                    prettyPrintAtom( r -> left ) ;
                } // if()
                else {
                    cout << "(" << " " ;
                    curLevel += 2 ; // A new group, level up
                    prettyPrintSExp( r -> left, LEFT, curLevel ) ;
                    curLevel -= 2 ;  // End of a new group, level down
                } // else()
                
                return prettyPrintSExp( r -> right, RIGHT, curLevel ) ;
            } // if()
            else if ( dir == RIGHT ){
                if ( r -> left -> type != CONS ) {
                    printWhite( curLevel + 2 ) ;
                    prettyPrintAtom( r -> left ) ;
                } // if()
                else {
                    prettyPrintSExp( r -> left, LEFT, curLevel ) ;
                } // else()
                
                return prettyPrintSExp( r -> right, RIGHT, curLevel ) ;
            } // else()
            
            return prettyPrintSExp( r -> right, RIGHT, curLevel ) ;
            
        } // else()
        
    } // prettyPrint()
    
    void prettyPrint( Node* r ) {
        if ( r -> type == ATOM || r -> type == SPECIAL ) { // this S-exp is an atom
            prettyPrintAtom( r ) ;
        } // if()
        else if ( r -> right -> lex == "nil" ) {
            prettyPrintAtom( r -> left ) ;
            gIsEOF = true ;
        }  // else if()
        else {
            prettyPrintSExp( r, LEFT, 0 ) ;
        } // else()
    } // printTree()
    
    void buildTree() {
        // Substitude () with nil and put on the ( )
        if ( s.isATOM( copyList.root -> token ) ) {
            Node* leaf = new Node ;
            leaf -> lex = copyList.root -> token.str ;
            if ( copyList.root -> token.type == NIL || copyList.root -> token.type == T ) {
                leaf -> type = SPECIAL ;
            } // if()
            else {
                leaf -> type = ATOM ;
            } // else()
            leaf -> left = NULL ;
            leaf -> right = NULL ;
            root = leaf ;
            leaf -> parent = root ;
            
        } // if((
        else {
            translate( copyList.root, copyList.tail ) ;
            // copyList.printForward() ;
            
            root = build( copyList.root, copyList.tail ) ;
            
            // singleList.print() ;
            // copyList.printForward() ;
            // copyList.printBackforward() ;
        } // else()
    } // buildTree()
    
    Node* getRoot() {
        return root ;
    } // getRoor()

};

int main() {
    
    LexicalAnalyzer la ;
    SyntaxAnalyzer sa ;
    bool grammerCorrect = false ;
    
    cout << "Welcome to OurScheme!" << endl ;
    string inputStr = "" ;

    try {
        while ( !gIsEOF ) {
            cout << "> " ;
            la.peekToken() ;
            Token token = la.getToken() ;
              
            grammerCorrect = sa.checkSExp( token ) ;
            if ( grammerCorrect ) {
                Tree tree( singleList ) ;
                tree.buildTree() ;
                tree.prettyPrint( tree.getRoot() ) ;
            } // if()
            
            singleList.clear() ;
        } // while()
    } catch ( MissingAtomOrLeftParException e ) {
        cout << e.err_mesg() << endl ;
    } catch ( MissingRightParException e ) {
        cout << e.err_mesg() << endl ;
    } catch ( NoClosingQuoteException e ) {
        cout << e.err_mesg() << endl ;
    } catch ( EOFException e ) {
        cout << e.err_mesg() << endl ;
    }
    
    cout << endl << "Thanks for using OurScheme!" << endl ;

} // main()
