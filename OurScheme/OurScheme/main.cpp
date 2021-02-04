# include <iostream>
# include <string>
# include <stdlib.h>

using namespace std ;

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

struct Token {
    string str ;  // the original apperance read from input
    int line ;  // the line which this token exist
    int column ;  // the column where this token exist
    TokenType type ;  // type of the token
    Token *next ; // point to the next token
    
    Token() {
        str = "" ;
        line = 0 ;
        column = 0 ;
        
    } // constructor
    
    void setInfo( string tokenStr,  int ln, int col, TokenType tType ) {
        str = tokenStr ;
        line = ln ;
        column = col ;
        type = tType ;
    } //setInfo()
} ;

struct Expression {
    Token *tokenList ;
    Expression *next ;  // pointed to the next expression
} ; // Expression

typedef Expression *ExpressionList ;  // a head that point to the first token of the expression

static int gLine = 1 ;
static int gColumn = 1 ;

static bool gTerminate = false ;  // used to check whether Our scheme has to be stop.

//  convert the enum value into the string, which will be more convenient to recognize
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

class LexicalAnalyzer {
private:
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
    
    bool isSeparator( char ch ) {
        if ( ch == '(' || ch == ')' || ch == '\'' || ch == '\"' || ch == ';' ) {
            return true ;
        } // if()
        
        return false ;
    } // isSepatator()
    
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
    
    void printToken( Token token ) {
        cout << "Token: >" << token.str << "<   Line: " << token.line << " Column: " << token.column << "type: " << enumToStr( token.type ) << endl ;
    } // printToken()
    
    // Purpose: since the original get token only get a token, but a string can contains many word, so need a special to process
    string getFullStr( string fullStr ) {
        // assert: 'ch' must be a peeked char which is '\"'
        char ch = '\0' ;
        
        ch = cin.peek() ;
        // because we need to get a string, keep reading the input until encounter the next '\"' or return-line
        while ( ch != '\"' && !isReturnLine( ch ) ) {
            ch = getChar() ;
            fullStr += ch ;
            ch = cin.peek() ;
        } // while()
        
        if ( ch == '\"' ) {  // a complete string with a correct syntax
            ch = getChar() ;
            fullStr += ch ;
        } // if()
        
        return fullStr ;
    } // getFullStr()
    
    // Purpose: responcible for getting next token, add keep the next char after the token unread
    // Return: (String) token string
    Token getToken() {
        string tokenStrWeGet = "" ;
        char ch = '\0' ;
        Token token ;
        
        ch = cin.peek() ;  // peek whether the next char is in input
        while ( isWhiteSpace( ch ) ) {  // before get a actual char, we need to skip all the white-spaces first
            ch = getChar() ;  // take away this white-space
            ch = cin.peek() ;  // keep peeking next char
        } // while()
        
        // assert: finally get a char which is not a white-space, now can start to construct a token
        ch = getChar() ;  // since this char is not a white-space, we can get it from the input
        tokenStrWeGet += ch ;  // directly add the first non-white-space char into the token string
        
        // if this char is already a separator then STOP reading, or keep getting the next char
        if ( !isSeparator( ch ) ) {  // 'ch' here is the first char overall
            ch = cin.peek() ;
            
            while ( !isSeparator( ch ) && !isWhiteSpace( ch ) ) {
                ch = getChar() ;
                tokenStrWeGet += ch ;
                ch = cin.peek() ;
            } // while()
        } // if()
        else if ( ch == '\"' ) {  // special case: this is the start of a string, call func. getFullStr()
            tokenStrWeGet = getFullStr( tokenStrWeGet ) ;
        } // else if()
        
        // assert: we get the whole token string
        
        token.setInfo( tokenStrWeGet, gLine, gColumn - ( int ) tokenStrWeGet.length(), findToken( tokenStrWeGet ) ) ;
        
        return token ;
    } // getToken()
    
    // Purpose: recognize whether this string is a INT
    // Return: true or false
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
    
    // Purpose: accept the token string from func. getToken(), and response the corresponding token value
    TokenType findToken( string str ) {
        
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
    } // findToken()
    
    public :
    void readExp( Expression expr ) {
        
    }  // ReadExp()
    
    void printExp( Expression expr ) {
        
    } // printExp()
    
    void testGetToken() {
        Token inputToken ;
        inputToken = getToken() ;
        while ( inputToken.str != "exit" ) {
            printToken( inputToken ) ; // test
            inputToken = getToken() ;
        } // while()
    } // testGetToken()
    
} ;

int main() {
    LexicalAnalyzer la ;
    cout << "Welcome to OurScheme!" << endl ;
    string inputStr = "" ;
    
    cout << "***Testing GetToken***" << endl ;
    la.testGetToken() ;
    
    
    /*
     while ( inputStr != "(EOF)" ) {
     
     cout << ">" ;
     cin >> inputStr ;
     // ReadExp( inputStr ) ;
     
     } // while()
     */
    
    cout << endl << "Thanks for using OurScheme!" ;
    
} // main()
