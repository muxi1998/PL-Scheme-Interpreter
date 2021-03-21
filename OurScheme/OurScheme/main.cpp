# include <iostream>
# include <string>
# include <stdlib.h>
# include <vector>
# include <stack>
# include <exception>

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

vector<Token> gTokenList ; // used to store the original order of the tokens from input

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
    
    Node_Linear(): next(NULL) {} ;
} ;

string intToStr( int num ) {
    string str = "" ;
    if ( num == 0 ) return "0" ;
    
    while ( num != 0 ) {
        str = ( char ) ( '0' + ( num % 10 ) ) + str ;
        num /= 10 ;
    } // while()
    
    return str ;
} // intToStr()

class MissingAtomOrLeftParException : public exception {
private:
    int line ;
    int col ;
    string str ;
    
public:
    MissingAtomOrLeftParException( int l, int c, string s ) : line( l ), col( c ), str( s ) {}
    string err_mesg() {
        string mesg = "" ;
        mesg = "ERROR (unexpected token) : atom or '(' expected when token at Line " + intToStr( line ) + " Column " + intToStr( col ) + " is >>" + str + "<<" ;
        
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
        mesg = "ERROR (unexpected token) : ')' expected when token at Line " + intToStr( line ) + " Column " + intToStr( col ) + " is >>" + str + "<<" ;
        
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
        mesg = "ERROR (no closing quote) : END-OF-LINE encountered at Line " + intToStr( line ) + " Column " + intToStr( col ) ;
        
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
        } // if()
        else {
            bool addSuccess = false ;
            
            for ( Node_Linear* walk = root ; walk != NULL && !addSuccess ; walk = walk -> next ) {
                if ( walk -> next == NULL ) {
                    walk -> next = newNode ;
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
            root = newNode ;
        } // if()
        else {
            newNode -> next = nodeBefore -> next ;
            nodeBefore -> next = newNode ;
        } // else()
        
    } // insertNode()
    
    void print() {
        for ( Node_Linear* walk = root ; walk != NULL ; walk = walk -> next ) {
            cout << walk -> token.str << "  (" << walk -> token.line << ", " << walk -> token.column << " ) " << enumToStr( walk -> token.type ) << endl ;
        } // for()
        cout << endl ;
        for ( Node_Linear* walk = root ; walk != NULL ; walk = walk -> next ) {
            cout << walk -> token.str << " " ;
        } // for()
    } // print()
    
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
    bool isFLOAT( string str ) {
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
        token.str = lex ;
        token.line = gLine ;
        token.column = gColumn - ( int ) lex.length() + 1 ;
        token.type = getTokenType( lex ) ;
        
        return token ;
    } // setTokenInfo()

public:
    
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
        // gTokenList.push_back( tokenWeWant ) ;
        singleList.addNode( tokenWeWant ) ;

        return tokenWeWant ;
    } // getToken()

};

// Purpose: Check the statement, if nothin wrong the build the tree, else print the error
class SyntaxAnalyzer {
    
private:
    
    LexicalAnalyzer la ;
    
    // Purpose: recognize whether this string is a INT
    // Return: true or false
    
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

public:
    
    void test() {
        string str = "" ;
        string getTokenStr = "" ;
        bool correct = true ;
        
        cout << "***Start test***" << endl ;
        la.peekToken() ;
        Token token = la.getToken() ;
        
        correct = checkSExp( token ) ;
        if ( correct ) {
            cout << "Grammer correct!" << endl ;
        } // while()
        else cout << "FAIL!!!!" << endl ;
        
        cout << "*** Print Lsit ***" << endl ;
        /*
        for ( int i = 0 ; i < gTokenList.size() ; i ++ ) {
            cout << "( " << gTokenList[ i ].line << ", " << gTokenList[ i ].column << " ) >" << gTokenList[ i ].str << "<" << enumToStr( gTokenList[ i ].type ) << endl ;
        } // for()
         */
        singleList.print() ;
    } // test()

} ;

struct Node {
    string lex ; // the string (what it looks in the input file) of this token
    NodeType type ; // Three possibility: 1.Atom  2.Special(NIL)  3.Cons
    Node* left ;
    Node* right ;

    Node() : lex(""), type(EMPTY), left(NULL), right(NULL) {} ;
};

class Tree {
    
private:
    
    Node *root ;
    
    void transferNIL( ) {
        bool finish = false ;
        
        if ( singleList.root -> token.type == LPAREN && singleList.root -> next -> token.type == RPAREN ) {
            Node_Linear* nilNode = new Node_Linear ;
            nilNode -> token.str = "nil" ;
            nilNode -> token.type = NIL ;
            nilNode -> token.line = singleList.root -> token.line ;
            nilNode -> token.column = singleList.root -> token.column ;
            
            nilNode -> next = singleList.root -> next -> next ;
            
            // Delete >)<
            delete singleList.root -> next ;
            singleList.root -> next = NULL ;
            // Delete >(<
            delete singleList.root ;
            
            singleList.root = nilNode ;
        } // if()
        
        for ( Node_Linear* walk = singleList.root ; walk -> next != NULL && walk -> next -> next != NULL && !finish ; walk = walk -> next ) {
            
            if ( walk -> next -> token.type == LPAREN && walk -> next -> next -> token.type == RPAREN ) {
                Node_Linear* nilNode = new Node_Linear ;
                nilNode -> token.str = "nil" ;
                nilNode -> token.type = NIL ;
                nilNode -> token.line = walk -> next -> token.line ;
                nilNode -> token.column = walk -> next -> token.column ;
                
                nilNode -> next = walk -> next -> next -> next ;
                
                // Delete >)<
                delete walk -> next -> next ;
                walk -> next -> next = NULL ;
                // Delete >(<
                delete walk -> next ;
                walk -> next = nilNode ;
            } // if()
            
        } // for()
        
    } // transferNIL()
    
    Node_Linear* findStrAndGetPreviousNode( string str ) {
        
        Node_Linear* target = NULL ;
        
        if ( str == singleList.root -> token.str ) {
            target = singleList.root ;
            return target ;
        } // if()
        
        for ( Node_Linear* walk = singleList.root ; walk -> next != NULL && target == NULL ; walk = walk -> next ) {
            if ( str == walk -> next -> token.str ) {
                target = walk ;
            } // if()
        } // for()
        
        return target ;
        
    } // findStrAndGetPreviousNode()
    
    void translate() {
        
        transferNIL() ;
        
        if ( singleList.root -> token.type != LPAREN ) { // this S-exp is not a list, so don't need to make up
            return ;
        } //if()
        
        stack<Node_Linear> nodeStack ; // used to find the dot which we need to pair up
        
        if ( findStrAndGetPreviousNode( "." ) == NULL ) { // there is no DOT, so put it on manually
            
        } // if()
        
    } // translate
    
public:
    
    Tree() : root(0) {} ;
    
    // Purpose: Transfer the DS from vector to pointer (tree)
    // Pre-request: tokens in vector construct a S-exp with correct grammer
    void build() {
        
    } // build()

    void printTree() {
        
    } // printTree()

    void prettyPrint() {
    } // prettyPrint()
    
    void test() {
        translate() ;
        
        singleList.print() ;
    } // test()

};

int main() {
    
    SyntaxAnalyzer sa ;
    Tree tree ;
    cout << "Welcome to OurScheme!" << endl ;
    string inputStr = "" ;

    try {
        
        sa.test() ;
        cout << endl << "** transfer ***" << endl ;
        tree.test() ;
        /*
        while ( inputStr != "(EOF)" ) {

          cout << ">" ;
          cin >> inputStr ;
          // ReadExp( inputStr ) ;

        } // while()
        */
        
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
