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

enum TokenType {
  LPAREN = 1067, RPAREN = 2134, INT = 1164, STRING = 1358, DOT = 3201,
  FLOAT = 1552, NIL = 1261, T = 2522, QUOTE = 4268, SYMBOL = 1746
} ;

enum NodeType { EMPTY = 0, ATOM = 1, CONS = 2, SPECIAL = 3 } ;

static int uTestNum = 0 ;  // test num from PAL
int gLine = 1 ;  // the line of the token we recently "GET"
int gColumn = 0 ; // // the golumn of the token we recently "GET"
string gPeekToken = "" ;  // the recent token we peek BUT haven't "GET"
bool gIsEOF = false ; // if is TRUE means there doesn't have '(exit)'

struct Token {
  string str ;  // the original apperance read from input
  int line ;  // the line which this token exist
  int column ;  // the column where this token exist
  TokenType type ;  // type of the token

  // Token(): str( "" ), line( 0 ), column( 0 ) {} ; // constructor
  /*
  Token() {
      str = "" ;
      line = 0 ;
      column = 0 ;
  } // reset()
  */
} ;

string EnumToStr( TokenType type ) {
  if ( type == LPAREN ) return "LPAREN" ;
  else if ( type == RPAREN ) return "RPAREN" ;
  else if ( type == INT ) return "INT" ;
  else if ( type == STRING ) return "STRING" ;
  else if ( type == DOT ) return "DOT" ;
  else if ( type == FLOAT ) return "FLOAT" ;
  else if ( type == NIL ) return "NIL" ;
  else if ( type == T ) return "T" ;
  else if ( type == QUOTE ) return "QUOTE" ;
  else if ( type == SYMBOL ) return "SYMBOL" ;
  
  return "" ;
} // EnumToStr()


struct Node_Linear {
  Token token ;
  Node_Linear* next ;
  Node_Linear* prev ;
    
    //  Node_Linear(): next( NULL ), prev( NULL ) {} ;
} ;

class SingleList {
    
public:
    
  Node_Linear* mRoot ;
  Node_Linear* mTail ;
    
  Node_Linear* FindNode( Token token ) {
    Node_Linear* nodeWeWant = NULL ;
        
    for ( Node_Linear* walk = mRoot ; walk != NULL && nodeWeWant != NULL ; walk = walk -> next ) {
      if ( token.str == walk -> token.str && token.line == walk -> token.line
           && token.column == walk -> token.column ) {
        nodeWeWant = walk ;
      } // if()
    } // for()
        
    return nodeWeWant ;
  } // FindNode()
    
  // Purpose: Simply add a new node at the tail
  void AddNode( Token token ) {
    Node_Linear* newNode = new Node_Linear ;
    newNode -> token = token ;
    newNode -> prev = NULL ;
    newNode -> next = NULL ;
        
    if ( mRoot == NULL ) { // empty
      mRoot = newNode ;
            // newNode -> prev = mRoot ;
      mTail = newNode ;
    } // if()
    else {
      bool addSuccess = false ;
            
      for ( Node_Linear* walk = mRoot ; walk != NULL && !addSuccess ; walk = walk -> next ) {
        if ( walk -> next == NULL ) {
          walk -> next = newNode ;
          newNode -> prev = walk ;
          mTail = newNode ;
          addSuccess = true ;
        } // if()
      } // for()
    } // else()
        
  } // AddNode()
    
  // Purpose: used to make up some DOT and () and NIL
  void InsertNode( Node_Linear* nodeBefore, TokenType type ) {
    Token token ;
    token.str  = "" ;
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
    else if ( type == QUOTE ) {
      token.str = "quote" ;
    } // else if()
        
    // assert: all information for this token has done
        
    Node_Linear* newNode = new Node_Linear ;
    newNode -> token = token ;
    newNode -> prev = NULL ;
    newNode -> next = NULL ;
    if ( nodeBefore == mRoot ) {
      newNode -> next = mRoot ;
      newNode -> prev = mRoot ;
      newNode -> next -> prev = newNode ;
      mRoot = newNode ;
    } // if()
    else {
      newNode -> next = nodeBefore -> next ;
      newNode -> prev = nodeBefore ;
      newNode -> next -> prev = newNode ;
      nodeBefore -> next = newNode ;
    } // else()
        
  } // InsertNode()
    
  void Print() {
    for ( Node_Linear* walk = mRoot ; walk != NULL ; walk = walk -> next ) {
      cout << walk -> token.str << "  (" << walk -> token.line ;
      cout << ", " << walk -> token.column << " ) " << EnumToStr( walk -> token.type ) << endl ;
    } // for()
    
    cout << endl ;
  } // Print()
    
  void PrintForward() {
    cout << endl << "*** Print forward ***" << endl ;
    for ( Node_Linear* walk = mRoot ; walk != NULL ; walk = walk -> next ) {
      cout << walk -> token.str << " " ;
    } // for()
  } // PrintForward()
    
  void PrintBackforward() {
    bool finish = false ;
    cout << endl << "*** Print backward ***" << endl ;
    for ( Node_Linear* walk = mTail ; !finish ; walk = walk -> prev ) {
      cout << walk -> token.str << " " ;
      if ( walk == mRoot ) finish = true ;
    } // for()
  } // PrintBackforward()
    
  void Clear() {
    while ( mRoot != NULL ) {
      Node_Linear* current = mRoot ;
      mRoot = mRoot -> next ;
      delete current ;
      current = NULL ;
    } // while()
    
    mRoot = NULL ;
  } // Clear()
    
} ;

SingleList gOriginalList ;

class GlobalFunction { // the functions that may be used in anywhere
    
public:
  string IntToStr( int num ) {
    string str = "" ;
    if ( num == 0 ) return "0" ;
        
    while ( num != 0 ) {
      str = ( char ) ( '0' + ( num % 10 ) ) + str ;
      num /= 10 ;
    } // while()
        
    return str ;
  } // IntToStr()
    
  int GetValueOfIntStr( string str ) {
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
  } // GetValueOfIntStr()
    
  float GetValueOfFloatStr( string str ) {
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
  } // GetValueOfFloatStr()
    
  void Reset() {
    gLine = 1 ;
    gColumn = 0 ;
    gOriginalList.Clear() ;
  } // Reset()
    
  void SkipLine() {
    char ch = cin.peek() ;
    while ( ch != '\n' ) {
      ch = cin.get() ;
      ch = cin.peek() ;
    } // while()
        
    gPeekToken = "" ;
  } // SkipLine()
    
  void PrintStr( string str ) {
    // \n \" \t \\
        
    for ( int i = 0 ; i < str.length() ; i ++ ) {
      // this char is a '\\' and still has next char  behind
      if ( str[ i ] == '\\' && i < str.length() - 1 ) {
        i ++ ; // skip '\\'
        if ( str[ i ] == 'n' ) {
          cout << endl ;
        } // if()
        else if ( str[ i ] == 't' ) {
          cout << '\t' ;
        } // else if()
        else if  ( str[ i ] == '"' ) {
          cout << '"' ;
        } // else if()
        else if ( str[ i ] == '\\' ) {
          cout << '\\' ;
        } // else if()
        else { // simple '\'
          cout << str[ i ] ;
          i -- ;
        } // else()
        // i ++ ; // skip the char right behind '\\'
      } // if()
      else {
        cout << str[ i ] ;
      } // else()
    } // for()
        
    cout << endl ;
  } // PrintStr()
    
} ;

GlobalFunction g ;

class MissingAtomOrLeftParException {
private:
  int mLine ;
  int mCol ;
  string mStr ;
    
public:
  MissingAtomOrLeftParException( int l, int c, string s ) {
    mLine = l ;
    mCol = c ;
    mStr = s ;
  } // MissingAtomOrLeftParException()
    
  string Err_mesg() {
    string mesg = "" ;
    mesg = "ERROR (unexpected token) : atom or '(' expected when token at Line " + g.IntToStr( mLine )
    + " Column " + g.IntToStr( mCol - ( int ) mStr.length() + 1 ) + " is >>" + mStr + "<<" ;
    g.SkipLine() ;
    int tmpGLine = gLine ;
    g.Reset() ;
    gLine = tmpGLine ;
        
    return mesg ;
  } // Err_mesg()
} ; // MissingAtomOrLeftParException

class MissingRightParException {
private:
  int mLine ;
  int mCol ;
  string mStr ;
    
public:
  MissingRightParException( int l, int c, string s ) {
    mLine = l ;
    mCol = c ;
    mStr = s ;
  } // MissingRightParException()
    
  string Err_mesg() {
    string mesg = "" ;
    mesg = "ERROR (unexpected token) : ')' expected when token at Line " + g.IntToStr( mLine )
    + " Column " + g.IntToStr( mCol - ( int ) mStr.length() + 1 ) + " is >>" + mStr + "<<" ;
    g.SkipLine() ;
    int tmpGLine = gLine ;
    g.Reset() ;
    gLine = tmpGLine ;
        
    return mesg ;
  } // Err_mesg()
} ; // MissingRightParException

class NoClosingQuoteException {
private:
  int mLine ;
  int mCol ;
    
public:
  NoClosingQuoteException( int l, int c ) {
    mLine = l ;
    mCol = c ;
  } // NoClosingQuoteException()
    
  string Err_mesg() {
    string mesg = "" ;
    mesg = "ERROR (no closing quote) : END-OF-LINE encountered at Line " + g.IntToStr( mLine )
    + " Column " + g.IntToStr( mCol ) ;
    int tmpGLine = gLine ;
    g.Reset() ;
    gLine = tmpGLine ;
        
    return mesg ;
  } // Err_mesg()
} ; // NoClosingQuoteException

class EOFException {
public:
  string Err_mesg() {
    string mesg = "" ;
    mesg = "ERROR (no more input) : END-OF-FILE encountered" ;
    int tmpGLine = gLine ;
    g.Reset() ;
    gLine = tmpGLine ;
        
    return mesg ;
  } // Err_mesg()
} ; // EOFException

class LexicalAnalyzer {
    
private:

  bool IsSeparator( char ch ) {
    if ( ch == '(' || ch == ')' || ch == '\'' || ch == '\"' || ch == ';' ) {
      return true ;
    } // if()

    return false ;
  } // IsSeparator()
    
  bool IsReturnLine( char ch ) {
    // Because Linux has '\r' character, I added '\r' as one circumstance
    if ( ch == '\n' || ch == '\r' ) {
      return true ;
    } // if()
            
    return false ;
  } // IsReturnLine()
    
  bool IsWhiteSpace( char ch ) {
    // Because Linux has '\r' character, I added '\r' as one circumstance
    if ( ch == ' ' || ch == '\t' || IsReturnLine( ch ) ) {
      return true ;
    } // if()

    return false ;
  } // IsWhiteSpace()
    
  string FormatFloat( string str ) {
    string formatStr = "" ;
        
    if ( str[ str.length() - 1 ] == '.' ) { // float num end with a dot
      formatStr = str + "000" ; // put some zero in it
    } // if()
    else if ( str[ 0 ] == '.' ) { // float num start with the dot
      formatStr = "0" + str ;
    } // else if()
    else {
      int dotIndex = ( int ) str.find( '.' ) ;
      int count = ( int ) str.length() - dotIndex ;
      formatStr = str ;
      for ( int i = 0 ; i < count ; i ++ ) {
        formatStr = formatStr + "0" ;
      } // for()
            
    } // else()
        
    return formatStr ;
  } // FormatFloat()
        
  // Purpose: not only call the func. cin.get(), but also increase the column or line
  char GetChar() {
    char ch = '\0' ;
    ch = cin.get() ;
    if ( IsReturnLine( ch ) ) {
      gLine ++ ;
      gColumn = 0 ;
    } // if()
    else  {
      gColumn ++ ;
    } // else()
        
    return ch ;
  } // GetChar()
    
  string GetFullStr( string fullStr ) {
    // assert: 'ch' must be a peeked char which is '\"'
    bool keepRead = true ;
    char ch_get = '\0' ;
    char ch_peek = '\0' ;
            
    ch_peek = cin.peek() ;
    // because we need to get a string, keep reading the input until
    // encounter the next '\"' or return-line
    while ( keepRead && !IsReturnLine( ch_peek ) ) {
      ch_get = GetChar() ;
      fullStr += ch_get ;
      ch_peek = cin.peek() ;
            
      if ( ch_peek == '\"' && ch_get != '\\' )  { // >"< stands alone
        keepRead = false ;
      } // if()
    } // while()
            
    if ( ch_peek == '\"' ) {  // a complete string with a correct syntax
      ch_get = GetChar() ;
      fullStr += ch_get ;
    } // if()
    else { // miss the ending quote
      throw NoClosingQuoteException( gLine, gColumn + 1 ) ;
    } // else()
        
    return fullStr ;
  } // GetFullStr()
    
  Token LexToToken( string lex ) {
    Token token ;
    token.str = "" ;
    token.line = 0 ;
    token.column = 0 ;
    if ( lex != "." && IsFLOAT( lex ) ) {
      // assert: float with a original format
      // now  can start trandfer the float into the format which (int).(3 chars)
      token.str = FormatFloat( lex ) ;
    } // if()
    else {
      token.str = lex ;
    } // else()
    
    token.line = gLine ;
    token.column = gColumn - ( int ) lex.length() + 1 ;
    token.type = GetTokenType( lex ) ;
        
    return token ;
  } // LexToToken()

public:
    
  bool IsINT( string str ) {
    // Mark1: there might be a sign char, such as '+' or '-'
    // Mark2: except the sign char, other char should be a number
    // Mark3: the  whole string cannot contain the dot
    int startIndex = 0 ;  // to avoid the sign char if there has one
            
    if ( str[ 0 ] == '+' || str[ 0 ] == '-' ) {
      startIndex = 1 ;  // the checking process start after the sign char
    } // if()
            
    for ( int i = startIndex ;  i < str.length() ; i ++ ) {
      if ( ! ( str[ i ] <= '9' && str[ i ] >= '0' ) ) {
        return false ;
      } // if()
    } // for()
            
    return true ;
  } // IsINT()
    
  // Purpose: recognize whether this string is a FLOAT
  // Return: true or false
  bool IsFLOAT( string &str ) {
    // Mark1: there might be a sign char, such as '+' or '-'
    // Mark2: except the sign char, other char should be a number
    // Mark3: the whole string SHOULD contain the dot,
    // NO MATTER the position of the dot is, but should be only dot
    // Mark4: if there appear another dot after already get one,
    // then might be a SYMBOL
    int dotNum = 0 ;  // only can have ONE dot
    int startIndex = 0 ;  // the checking process start after the sign char
        
    if ( str[ 0 ] == '+' || str[ 0 ] == '-' ) {
      startIndex = 1 ;  // the checking process start after the sign char
    } // if()
            
    for ( int i = startIndex ;  i < str.length() ; i ++ ) {
      if ( str[ i ] == '.' ) {
        dotNum ++ ;  // every time we encounter a dot, count it
      } // if()
                
      if ( ! ( str[ i ] <= '9' && str[ i ] >= '0' ) && str[ i ] != '.' ) {
        return false ;
      } // if()
    } // for()
            
    if ( dotNum != 1 ) {
      return false ;
    } // if()
        
    return true ;
  } // IsFLOAT()
    
  bool IsStr( string str ) {
    if ( str[ 0 ] == '"' && str[ str.length() - 1 ] == '"' ) {
      return true ;
    } // if()
        
    return false ;
  } // IsStr()
    
  // Purpose: accept the token string from func. GetToken(), and response the corresponding token value
  TokenType GetTokenType( string str ) {
        
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
    else if ( IsINT( str ) ) {
      return INT ;
    } // else if()
    else if ( IsFLOAT( str ) ) {
      return FLOAT ;
    } // else if()
    else if ( str[ 0 ] == '\"' ) {
      return STRING ;
    } // else if()
        
    return SYMBOL ;  // none of the above, then assume it's symbol
  } // GetTokenType()
    
  string PeekToken() {
    string tokenStrWeGet = "" ;
        
    if ( gPeekToken == "" ) {
      char ch = '\0' ;

      ch = cin.peek() ;  // peek whether the next char is in input
      if ( ch == -1 ) { // -1 means EOF for cin.peek
        gIsEOF = true ;
        throw EOFException() ;
      } // if()
            
      // before get a actual char, we need to skip all the white-spaces first
      while ( IsWhiteSpace( ch ) ) {
        ch = cin.get() ;  // take away this white-space
        gColumn ++ ;
                
        if ( IsReturnLine( ch ) ) {
          gLine ++ ;
          gColumn = 0 ;
        } // if()
                
        ch = cin.peek() ;  // keep peeking next char
      } // while()

      // assert: finally get a char which is not a white-space, now can start to construct a token
      ch = GetChar() ;  // since this char is not a white-space, we can get it from the input
      tokenStrWeGet += ch ;  // directly add the first non-white-space char into the token string

      // if this char is already a separator then STOP reading, or keep getting the next char
      if ( !IsSeparator( ch ) && ch != '\"' ) {  // 'ch' here is the first char overall
        ch = cin.peek() ;

        while ( !IsSeparator( ch ) && !IsWhiteSpace( ch ) ) {
          ch = GetChar() ;
          tokenStrWeGet += ch ;
          ch = cin.peek() ;
        } // while()
                
      } // if()
      else if ( ch == '\"' ) {
        // assert: we get the whole token
        tokenStrWeGet = GetFullStr( tokenStrWeGet ) ;
      } // else if()

      // assert: we get the whole token
      gPeekToken = tokenStrWeGet ;
    } // if()
        
    return gPeekToken ;
        
  } // PeekToken()

  Token GetToken() {
    if ( gPeekToken == "" ) PeekToken() ;
        
    if ( gPeekToken == ";" ) { // encounter a line comment
      char ch = GetChar() ;
      while ( !IsReturnLine( ch ) ) {
        ch = GetChar() ;
      } // while()
      
      gLine ++ ;
      gColumn = 0 ;
      gPeekToken = "" ;
      PeekToken() ;
    } // if()
      
    Token tokenWeWant = LexToToken( gPeekToken ) ;
    gPeekToken = "" ;
    gOriginalList.AddNode( tokenWeWant ) ;

    return tokenWeWant ;
  } // GetToken()

};

// Purpose: Check the statement, if nothin wrong the build the tree, else Print the error
class SyntaxAnalyzer {
    
private:
    
  LexicalAnalyzer mLa ;
    
public:
    
  bool CheckSExp( Token startToken ) {
    // assert: startToken can only has three possibility
    // 1.Atom 2.LP 3.Quote *4.LR RP
        
    // this is a NIL with special format >()<
    if ( startToken.type == LPAREN && mLa.GetTokenType( mLa.PeekToken() ) == RPAREN ) {
      mLa.GetToken() ; // take away the RP from the input
            
      return true ; // one of an ATOM
    } // if()
    else if ( IsATOM( startToken ) ) {
      return true ;
    } // else if()
    else if ( startToken.type == QUOTE ) {
      Token token = mLa.GetToken() ; // get the next token, suppose to be the start of a S-exp
            
      return  CheckSExp( token ) ;
    } // else if()
    else if ( startToken.type == LPAREN ) {
      // suppose to have at least ONE S-exp
      bool hasOneSExpCorrect = false ;
      bool moreSExpCorrect = true ;
            
      Token token = mLa.GetToken() ; // get the next token, suppose to be the start of a S-exp
      hasOneSExpCorrect = CheckSExp( token ) ;
            
      if ( hasOneSExpCorrect ) {
                
        while ( ( mLa.GetTokenType( mLa.PeekToken() ) == LPAREN
                  || IsATOM( mLa.PeekToken() )
                  || mLa.GetTokenType( mLa.PeekToken() ) == QUOTE )
                && moreSExpCorrect ) {
          token = mLa.GetToken() ;
          moreSExpCorrect = CheckSExp( token ) ;
                    
          mLa.PeekToken() ; // maybe successfully check a correct S-exp, keep peeking the next one
        } // while()
                
        if ( !moreSExpCorrect ) { // there are more S-exp, but not all correct
          return false ;
        } // if()
        else {
          // means only one S-exp in this left S-exp
          if ( mLa.GetTokenType( mLa.PeekToken() ) == DOT ) {
            token = mLa.GetToken() ; // must be DOT
            // must be the start of the next S-exp according to the grammer
            token = mLa.GetToken() ;
                        
            hasOneSExpCorrect = CheckSExp( token ) ;
            if ( !hasOneSExpCorrect ) {
              throw MissingAtomOrLeftParException( gLine, gColumn, gPeekToken ) ;
              return false ;
            } // if()
            else {
              if ( mLa.GetTokenType( mLa.PeekToken() ) == RPAREN ) {
                token = mLa.GetToken() ; // must be >)<
                                
                return true ;
              } // if()
              else {
                throw MissingRightParException( gLine, gColumn, gPeekToken ) ;
                return false ;
              } // else()
            } // else()
                        
          } // if()
          else if ( mLa.GetTokenType( mLa.PeekToken() ) == RPAREN ) {
            token = mLa.GetToken() ; // must be >)<
                        
            return true ;
          } // else if()
        } // else()
      } // if()
      else {
        throw MissingAtomOrLeftParException( gLine, gColumn, token.str ) ;
      } // else()
            
      return false ;
      
    } // else if()
        
    throw MissingAtomOrLeftParException( gLine, gColumn, startToken.str ) ;
        
    return false ; // none of the above begining
        
  } // CheckSExp()

  bool IsATOM( Token token ) {
    TokenType type = token.type ;
    if ( type == SYMBOL || type == INT || type == FLOAT || type == STRING || type == NIL || type == T ) {
      return true ;
    } // if()
        
    return false ;
  } // IsATOM()
    
  bool IsATOM( string str ) {
    TokenType type = mLa.GetTokenType( str ) ;
    if ( type == SYMBOL || type == INT || type == FLOAT || type == STRING || type == NIL || type == T ) {
      return true ;
    } // if()
        
    return false ;
  } // IsATOM()

} ;

struct Node {
  string lex ; // the string (what it looks in the input file) of this token
  NodeType type ; // Three possibility: 1.Atom  2.Special(NIL)  3.Cons
  Node* left ;
  Node* right ;
  Node* parent ;

    // Node() : lex(""), type(EMPTY), left(NULL), right(NULL), parent(NULL) {} ;
};

class Tree {
    
private:
    
  Node *mRoot ;
  SingleList mCopyList ;
    
  LexicalAnalyzer mLa ;
  SyntaxAnalyzer mS ;
    
  enum Direction { RIGHT = 1234, LEFT = 4321 } ;
    
  void TransferNIL( Node_Linear* root, Node_Linear* tail ) {
    bool finish = false ;
        
    // only ()
    if ( root -> token.type == LPAREN && root -> next -> token.type == RPAREN ) {
      Node_Linear* nilNode = new Node_Linear ;
      nilNode -> prev = NULL ;
      nilNode -> next = NULL ;
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
        
    for ( Node_Linear* walk = root ;
          walk -> next != NULL && walk -> next -> next != NULL && !finish ;
          walk = walk -> next ) {
            
      if ( walk -> next -> token.type == LPAREN && walk -> next -> next -> token.type == RPAREN ) {
        Node_Linear* nilNode = new Node_Linear ;
        nilNode -> prev = NULL ;
        nilNode -> next = NULL ;
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
  } // TransferNIL()
    
  Node_Linear* FindStrAndGetPreviousNode( Node_Linear* root, Node_Linear* tail, string str ) {
        
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
        
  } // FindStrAndGetPreviousNode()
    
  Node_Linear* FindCorrespondPar( Node_Linear* par_L ) {
    stack<Node_Linear*> nodeStack ;
    Node_Linear* target = NULL ; // the pointer that pointed to the Right parathesis
        
    nodeStack.push( par_L ) ; // put the first left parathesis in the stack
    for ( Node_Linear* walk = par_L -> next ;
          target == NULL && !nodeStack.empty() ;
          walk = walk -> next ) {
            
      if ( walk -> token.type == RPAREN ) {
        // when encounter a right par,
        // then keep pop out the items util meet the first left par
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
  } // FindCorrespondPar()
    
  Node_Linear* FindDOT( Node_Linear* root, Node_Linear* tail ) {
    stack<Node_Linear*> s ;
    int count = 0 ;
        
    s.push( tail ) ;
    if ( tail -> prev != NULL && tail -> prev -> token.type != RPAREN ) {
      if (  tail -> prev -> prev != NULL && tail -> prev -> prev -> token.type != DOT ) {
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
      } // while()
            
      return s.top() ;
    } // if()
        
    return NULL ;
        
  } // FindDOT()
    
  // Purpose: focus on one S-exp and give it the parathesis
  // Only list can call this function
  void Translate( Node_Linear* root, Node_Linear* tail ) {
        
    if ( mCopyList.mRoot -> token.type == LPAREN
         && mCopyList.mRoot -> next -> token.type == RPAREN
         && mCopyList.mRoot -> next -> next == NULL ) {
          
      Node_Linear* nilNode = new Node_Linear ;
      nilNode -> prev = NULL ;
      nilNode -> next = NULL ;
      nilNode -> token.str = "nil" ;
      nilNode -> token.type = NIL ;
      nilNode -> token.line = mCopyList.mRoot -> token.line ;
      nilNode -> token.column = mCopyList.mRoot -> token.column ;
      nilNode -> next = NULL ;
            
      while ( mCopyList.mRoot != NULL ) { // Clear ( )
        Node_Linear* current = mCopyList.mRoot ;
        mCopyList.mRoot = mCopyList.mRoot -> next ;
        delete current ;
        current = NULL ;
      } // while()
            
      mCopyList.mRoot = nilNode ; // connect nil node
      mCopyList.mTail = mCopyList.mRoot ;
      gOriginalList.mRoot = mCopyList.mRoot ;
      mCopyList.mTail = gOriginalList.mRoot ;
      return ;
    } // if()
    else {
      TransferNIL( root, tail ) ; // put a NIL in this list if needed
    } // else()
        
    int countPar = 0 ; // increase when manually add DOT and Paranthesis
        
    Node_Linear* dotPointer = FindDOT( root, tail ) ;
    if ( dotPointer == NULL ) { // there is no DOT, so put it on manually
      // assert: there is no DOT so need to add DOT and nil
      mCopyList.InsertNode( tail -> prev, DOT ) ;
      mCopyList.InsertNode( tail -> prev, NIL ) ;
    } // if()
        
    // assert: there must be at least one dot in this S-exp
    bool hasFinish = false ;
    for ( Node_Linear* walk = root -> next ; !hasFinish && walk != tail ; ) {
      if ( walk -> token.type == LPAREN ) {
        Node_Linear* corRightPar = FindCorrespondPar( walk ) ;
        Translate( walk, corRightPar ) ;
        walk = corRightPar ;
      } // if()
            
      if ( walk -> next -> token.type != DOT ) { // manually add DOT and keep counting
        mCopyList.InsertNode( walk, DOT ) ;
        walk = walk -> next ;
        mCopyList.InsertNode( walk, LPAREN ) ;
        walk = walk -> next ;
        countPar ++ ;
      } // if()
      else {
        walk = walk -> next ; // skip the atom before the DOT
        if ( walk -> next -> token.type == LPAREN ) {
          Node_Linear* corRightPar = FindCorrespondPar( walk -> next ) ;
          Translate( walk -> next, corRightPar ) ;
          walk = corRightPar ;
                    
          for ( int i = 0 ; i < countPar ; i ++ ) {
            mCopyList.InsertNode( walk, RPAREN ) ;
          } // for()
        } // if()
        else { // only an atom
          walk = walk -> next ; // skip DOT
          for ( int i = 0 ; i < countPar ; i ++ ) {
            mCopyList.InsertNode( walk, RPAREN ) ;
          } // for()
        } // else()
                
        hasFinish = true ;
      } // else()
            
      if ( walk != NULL ) walk = walk -> next ;
    } // for()
        
  } // Translate()
    
  void PrintWhite( int num ) {
    for ( int i = 0 ; i < num ; i ++ ) {
      cout << " " ;
    } // for()
  } // PrintWhite()
    
public:
  Tree( SingleList list ) {
    mRoot = NULL ;
    mCopyList = list ;
        
    if ( mCopyList.mRoot -> token.type == QUOTE ) {
      mCopyList.mRoot -> token.str = "quote" ;
      mCopyList.InsertNode( mCopyList.mRoot, LPAREN ) ;
      Token token ;
      token.str = ")" ;
      token.type = RPAREN ;
      token.line = -1 ;
      token.column = -1 ;
      mCopyList.AddNode( token ) ;
    } // if()
  } // Tree()
    
  // Purpose: Transfer the DS from list to pointer (tree)
  // Pre-request: tokens in vector construct a S-exp with correct grammer
  // Return the root of this tree
  Node* Build( Node_Linear* leftPointer, Node_Linear* rightPointer ) {
        
    if ( leftPointer -> token.type == LPAREN &&
         rightPointer -> token.type == DOT ) { // left part if the cons
      Node* atomNode = new Node ;
      atomNode -> lex = "" ;
      atomNode -> left = NULL ;
      atomNode -> right = NULL ;
      atomNode -> parent = NULL ;
      atomNode -> type = EMPTY ;
            
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
      atomNode -> lex = "" ;
      atomNode -> left = NULL ;
      atomNode -> right = NULL ;
      atomNode -> parent = NULL ;
      atomNode -> type = EMPTY ;
            
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
      Node_Linear* dotPointer = FindDOT( leftPointer, rightPointer ) ;
      Node* leftSubTree = Build( leftPointer -> next, dotPointer -> prev ) ;
      Node* rightSubTree = Build( dotPointer -> next, rightPointer -> prev ) ;
            
      Node* cons = new Node ;
      cons -> lex = "" ;
      cons -> left = NULL ;
      cons -> right = NULL ;
      cons -> parent = NULL ;
      cons -> type = EMPTY ;
            
      cons -> type = CONS ;
      cons -> left = leftSubTree ;
      cons -> left -> parent = cons ;
      cons -> right = rightSubTree ;
      cons -> right -> parent = cons ;
            
      return cons ;
    } // else()
        
    return NULL ;
  } // Build()

  void PrettyPrintAtom( Node* r ) {
    if ( r -> type == SPECIAL ) {
      if ( r -> lex == "nil" || r -> lex == "#f" ) {
        cout << "nil" << endl ;
      } // if()
      else {
        cout << "#t" << endl ;
      } // else()
    } // if()
    else if ( mLa.IsINT( r -> lex ) ) {
      cout << g.GetValueOfIntStr( r -> lex ) << endl ;
    } // else if()
    else if ( mLa.IsFLOAT( r -> lex ) ) {
      cout << fixed << setprecision( 3 ) << g.GetValueOfFloatStr( r -> lex ) << endl ;
    } // else if()
    else if ( mLa.IsStr( r -> lex ) ) {
      g.PrintStr( r -> lex ) ;
    } // else if()
    else cout << r -> lex << endl ;
  } // PrettyPrintAtom()
    
  void PrettyPrintSExp( Node* r, Direction dir, int level, bool rightPart ) {
        
    int curLevel = level ;
        
    if ( r -> type == ATOM || r -> type == SPECIAL ) {
            // case1. LL case2. RL case3. RR
      if ( dir == LEFT ) {
        PrintWhite( curLevel + 2 ) ;
        PrettyPrintAtom( r -> left ) ;
      } // if()
      else {
        if ( r -> lex != "nil" && r -> lex != "#f" ) {
          PrintWhite( curLevel + 2 ) ;
          cout << "." << endl ;
          PrintWhite( curLevel + 2 ) ;
          PrettyPrintAtom( r ) ;
        } // if()
                
        PrintWhite( curLevel ) ;
        cout << ")" << endl ;
      } // else()
            
      return ;
    } // if()
    else { // CONS node
            
      if ( dir == LEFT ) {
        if ( r -> left -> type != CONS ) {
          if ( rightPart ) {
            PrintWhite( curLevel ) ;
          } // if()
          
          cout << "(" << " " ;
          PrettyPrintAtom( r -> left ) ;
        } // if()
        else {
          if ( rightPart ) {
            PrintWhite( curLevel ) ;
          } // if()
          
          cout << "(" << " " ;
          curLevel += 2 ; // A new group, level up
          PrettyPrintSExp( r -> left, LEFT, curLevel, rightPart ) ;
          curLevel -= 2 ;  // End of a new group, level down
        } // else()
                
        return PrettyPrintSExp( r -> right, RIGHT, curLevel, true ) ;
      } // if()
      else if ( dir == RIGHT ) {
        if ( r -> left -> type != CONS ) {
          PrintWhite( curLevel + 2 ) ;
          PrettyPrintAtom( r -> left ) ;
        } // if()
        else {
          curLevel += 2 ; // A new group, level up
          PrettyPrintSExp( r -> left, LEFT, curLevel, rightPart ) ;
          curLevel -= 2 ;  // End of a new group, level down
        } // else()
                
        return PrettyPrintSExp( r -> right, RIGHT, curLevel, true ) ;
      } // else if()
            
      return PrettyPrintSExp( r -> right, RIGHT, curLevel, true ) ;
            
    } // else()
        
  } // PrettyPrintSExp()
    
  void PrettyPrint( Node* r ) {
    if ( r -> type == ATOM || r -> type == SPECIAL ) { // this S-exp is an atom
      PrettyPrintAtom( r ) ;
    } // if()
    else if ( r -> right -> lex == "nil" ) {
      PrettyPrintAtom( r -> left ) ;
    }  // else if()
    else {
      PrettyPrintSExp( r, LEFT, 0, false ) ;
    } // else()
  } // PrettyPrint()
    
  void BuildTree() {
    // Substitude () with nil and put on the ( )
    if ( mS.IsATOM( mCopyList.mRoot -> token ) ) {
      Node* leaf = new Node ;
      leaf -> lex = "" ;
      leaf -> left = NULL ;
      leaf -> right = NULL ;
      leaf -> parent = NULL ;
      leaf -> type = EMPTY ;
            
      leaf -> lex = mCopyList.mRoot -> token.str ;
      if ( mCopyList.mRoot -> token.type == NIL || mCopyList.mRoot -> token.type == T ) {
        leaf -> type = SPECIAL ;
      } // if()
      else {
        leaf -> type = ATOM ;
      } // else()
      
      leaf -> left = NULL ;
      leaf -> right = NULL ;
      mRoot = leaf ;
      leaf -> parent = mRoot ;
            
    } // if((
    else {
      Translate( mCopyList.mRoot, mCopyList.mTail ) ;
      // mCopyList.PrintForward() ;
            
      mRoot = Build( mCopyList.mRoot, mCopyList.mTail ) ;
            
      if ( mRoot -> type == CONS && mRoot -> left -> lex == "exit"
           && mRoot -> right -> lex == "nil" ) {
        gIsEOF = true ;
      } // if()
      // gOriginalList.Print() ;
      // mCopyList.PrintForward() ;
      // mCopyList.PrintBackforward() ;
    } // else()
  } // BuildTree()
    
  Node* GetRoot() {
    return mRoot ;
  } // GetRoot()

};

int main() {
    
  LexicalAnalyzer la ;
  SyntaxAnalyzer sa ;
  bool grammerCorrect = false ;
  
  cin >> uTestNum ;
  
  if ( uTestNum == 1 ) {
    cout << "Test1" << endl ;
    exit( 0 ) ;
  } // if()
    
  cout << "Welcome to OurScheme!" << endl ;
  string inputStr = "" ;
    
  while ( !gIsEOF ) {
        
    try {
            
      cout << "> " ;
      la.PeekToken() ;
      Token token = la.GetToken() ;
              
      grammerCorrect = sa.CheckSExp( token ) ;
      if ( grammerCorrect ) {
        Tree tree( gOriginalList ) ;
        tree.BuildTree() ;
        if ( !gIsEOF ) tree.PrettyPrint( tree.GetRoot() ) ;
      } // if()
            
      gOriginalList.Clear() ;
      g.Reset() ;
            
    } // catch()
    catch ( MissingAtomOrLeftParException e ) {
      cout << e.Err_mesg() << endl ;
    } // catch()
    catch ( MissingRightParException e ) {
      cout << e.Err_mesg() << endl ;
    } // catch()
    catch ( NoClosingQuoteException e ) {
      cout << e.Err_mesg() << endl ;
    } // catch()
    catch ( EOFException e ) {
      cout << e.Err_mesg() << endl ;
    } // catch()
        
  } // while()
    
  cout << endl << "Thanks for using OurScheme!" << endl ;

} // main()
