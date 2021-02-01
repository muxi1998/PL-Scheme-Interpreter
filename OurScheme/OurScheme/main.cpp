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

static int gLine = 0 ;
static int gColumn = 0 ;

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
} ;

typedef Token *TokenPtr ;  // a head that point to the first token of the expression

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
    bool isWhiteSpace( char ch ) {
      if ( ch == ' '|| ch == '\t' || ch == '\n' || ch == '\r' ) {  // Because Linux has '\r' character, I added '\r' as one circumstance
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

    // Purpose: responcible for getting next token, add keep the next char after the token unread
    // Return: (String) token string
    string getToken() {
      string tokenStrWeGet = "" ;
      char ch = '\0' ;
      
      ch = cin.peek() ;  // peek whether the next char is in input
      while ( isWhiteSpace( ch ) ) {  // before get a actual char, we need to skip all the white-spaces first
        ch = cin.get() ;  // take away this white-space
        ch = cin.peek() ;  // keep peeking next char
      } // while()

      // assert: finally get a char which is not a white-space, now can start to construct a token
      ch = cin.get() ;  // since this char is not a white-space, we can get it from the input
      tokenStrWeGet += ch ;  // directly add the first non-white-space char into the token string

      // if this char is already a separator then STOP reading, or keep getting the next char
      if ( !isSeparator( ch ) ) {  // 'ch' here is the first char overall
        ch = cin.peek() ;
        
        while ( !isSeparator( ch ) && !isWhiteSpace( ch ) ) {
          ch = cin.get() ;
          tokenStrWeGet += ch ;
          ch = cin.peek() ;
        } // while()
      } // if()
      
      // assert: we get the whole token
      return tokenStrWeGet ;

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
    
    bool isSTRING( string str ) {
        return true ;
    } // isSTRING()
    
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
        else if ( isSTRING( str ) ) {
            
        } // else if()
        
        return SYMBOL ;  // none of the above, then assume it's symbol
    } // findToken()

  public :
    void readExp( TokenPtr expr ) {
      
    }  // ReadExp()

    void printExp( TokenPtr expr ) {
       
    } // printExp()

    void testGetToken() {
      string inputStr = "" ;
      inputStr = getToken() ;
      while ( inputStr != "exit" ) {
        cout << "<" << inputStr << ">" << " Type: " << enumToStr( findToken( inputStr ) ) << endl ;
        inputStr = getToken() ;
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

} // main()
