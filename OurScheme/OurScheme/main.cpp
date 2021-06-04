# include <iostream>
# include <string>
# include <stdlib.h>
# include <vector>
# include <stack>
# include <exception>
# include <iomanip>
# include <sstream>

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
int gReserveWordNum = 51 ;
string gOriginReserveWordList[ 51 ] = { "cons", "list", "quote", "define"
  , "car", "cdr", "not", "and", "or", "begin", "if", "cond"
  ,  "clean-environment", "quote", "'", "atom?", "pair?", "list?"
  , "null?", "integer?", "real?", "number?", "string?",  "boolean?"
  , "symbol?", "+", "-", "*", "/", ">", ">=", "<", "<=", "=", "and"
  , "not", "or", "string-append", "string>?", "string<?", "string=?"
  , "eqv?", "equal?", "begin", "if", "cond", "exit", "lambda", "let", "verbose", "verbose?" } ;


enum TokenType {
  LPAREN = 1067, RPAREN = 2134, INT = 1164, STRING = 1358, DOT = 3201,
  FLOAT = 1552, NIL = 1261, T = 2522, QUOTE = 4268, SYMBOL = 1746
} ;

enum NodeType { EMPTY = 0, ATOM = 1, CONS = 2, SPECIAL = 3 } ;

struct Node {
  string lex ; // the string (what it looks in the input file) of this token
  NodeType type ; // Three possibility: 1.Atom  2.Special(NIL)  3.Cons
  Node* left ;
  Node* right ;
  Node* parent ;
  bool isAddByMe ;
  // Node() : lex(""), type(EMPTY), left(NULL), right(NULL), parent(NULL) {} ;
};

static int uTestNum = 0 ;  // test num from PAL
int gLine = 1 ;  // the line of the token we recently "GET"
int gColumn = 0 ; // // the golumn of the token we recently "GET"
string gPeekToken = "" ;  // the recent token we peek BUT haven't "GET"
bool gIsEOF = false ; // if is TRUE means there doesn't have '(exit)'
bool gJustFinishAExp = false ;
bool gVerbose = true ;
Node* gErrNode = NULL ;

struct Token {
  string str ;  // the original apperance read from input
  int line ;  // the line which this token exist
  int column ;  // the column where this token exist
  TokenType type ;  // type of the token
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
  bool isAddByMe ;
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
    newNode -> isAddByMe = false ;
    
    if ( mRoot == NULL ) { // empty
      mRoot = newNode ;
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
    newNode -> isAddByMe = false ;
    
    if ( type == NIL ) {
      newNode -> isAddByMe = true ; // additionly add the NIL node by myself
    } // if()
    
    if ( nodeBefore == NULL ) {
      newNode -> next = mRoot ;
      newNode -> prev = NULL ;
      newNode -> next -> prev = newNode ;
      mRoot = newNode ;
    } // if()
    else if ( nodeBefore == mRoot ) {
      newNode -> next = nodeBefore -> next ;
      newNode -> prev = nodeBefore ;
      newNode -> next -> prev = newNode ;
      nodeBefore -> next = newNode ;
    } // else if()
    else if ( nodeBefore == mTail ) {
      newNode -> next = nodeBefore -> next ;
      newNode -> prev = nodeBefore ;
      newNode -> next = NULL ;
      nodeBefore -> next = newNode ;
      mTail = newNode ;
    } // else if()
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
    
    delete mRoot ;
    mRoot = NULL ;
    mTail = NULL ;
  } // Clear()
  
} ;

SingleList gOriginalList ;

class GlobalFunction { // the functions that may be used in anywhere
private:
  enum Direction { RIGHT = 1234, LEFT = 4321 } ;
  
public:
  
  bool IsINT( string str ) {
    // Mark1: there might be a sign char, such as '+' or '-'
    // Mark2: except the sign char, other char should be a number
    // Mark3: the  whole string cannot contain the dot
    int startIndex = 0 ;  // to avoid the sign char if there has one
    bool hasNum = false ;
    
    if ( str[ 0 ] == '+' || str[ 0 ] == '-' ) {
      startIndex = 1 ;  // the checking process start after the sign char
    } // if()
    
    for ( int i = startIndex ;  i < str.length() ; i ++ ) {
      if ( ! ( str[ i ] <= '9' && str[ i ] >= '0' ) ) {
        return false ;
      } // if()
      else if ( str[ i ] <= '9' && str[ i ] >= '0' ) {
        hasNum = true ;
      } // else if()
    } // for()
    
    if ( ! hasNum ) {
      return false ;
    } // if()
    
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
    bool hasNum = false ;
    
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
      else if ( str[ i ] <= '9' && str[ i ] >= '0' ) {
        hasNum = true ;
      } // else if()
    } // for()
    
    if ( dotNum != 1 || !hasNum ) {
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
  
  bool IsSymbol( string str ) {
    if ( str != "" && GetTokenType( str ) == SYMBOL ) {
      for ( int i = 0 ; i < gReserveWordNum ; i ++ ) {
        if ( str == gOriginReserveWordList[ i ] ) {
          return false ;
        } // if()
      } // for()
      
      return true ;
    } // if()
    
    return false ;
  } // IsSymbol()
  
  string IntToStr( int num ) {
    string str = "" ;
    bool isNegative = false ;
    if ( num == 0 ) return "0" ;
    else if ( num < 0 ) {
      isNegative = true ;
      num *= -1 ; // change it to positive for calculating
    } // else if()
    
    while ( num != 0 ) {
      str = ( char ) ( '0' + ( num % 10 ) ) + str ;
      num /= 10 ;
    } // while()
    
    if ( isNegative ) {
      str = "-" + str ;
    } // if()
    
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
  
  double GetValueOfFloatStr( string str ) {
    double num = 0.0 ;
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
  
  string FormatIntToFloatStr( string str ) {
    return str += ".000" ;
  } // FormatIntToFloatStr()
  
  Node* GetNullNode() {
    Node* nullNode = new Node ;
    nullNode -> lex = "nil" ;
    nullNode -> type = SPECIAL ;
    nullNode -> parent = NULL ;
    nullNode -> left = NULL ;
    nullNode -> right = NULL ;
    nullNode -> isAddByMe = false ;
    
    return nullNode ;
  } // GetNullNode()
  
  Node* GetEmptyNode() {
    Node* nullNode = new Node ;
    nullNode -> lex = "" ;
    nullNode -> type = EMPTY ;
    nullNode -> parent = NULL ;
    nullNode -> left = NULL ;
    nullNode -> right = NULL ;
    nullNode -> isAddByMe = false ;
    
    return nullNode ;
  } // GetEmptyNode()
  
  void Reset() {
    gLine = 1 ;
    gColumn = 0 ;
    gOriginalList.Clear() ;
    gPeekToken = "" ;
    gErrNode = NULL ;
  } // Reset()
  
  void SkipLine() {
    char ch = '\0' ;
    ch = cin.peek() ;
    while ( ch != '\n' && !IsEOF( ch ) ) {
      ch = cin.get() ;
      ch = cin.peek() ;
    } // while()
    
    if ( !IsEOF( ch ) ) {
      ch = cin.get() ; // return-line
    } // if()
    
    gPeekToken = "" ;
    if ( gJustFinishAExp ) { // the following tokens are comment
      gLine = 1 ; // next line start from 1
      gJustFinishAExp = false ;
    } // if()
    else { // the comments in the middle of a S-exp
      gLine ++ ;
    } // else()
    
    gColumn = 0 ;
  } // SkipLine()
  
  string GetStrContent( string str ) {
    string newString = "" ;
    if ( str[ 0 ] == '"' ) {
      newString = str.substr( 1, str.length() - 2 ) ;
    } // if()
    else {
      newString = str ;
    } // else()
    
    return newString ;
  } // GetStrContent()
  
  void PrintStr( string str ) {
    
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
          i -- ;
          cout << str[ i ] ;
        } // else()
        // i ++ ; // skip the char right behind '\\'
      } // if()
      else {
        cout << str[ i ] ;
      } // else()
    } // for()
    
    cout << endl ;
  } // PrintStr()
  
  bool IsEOF( char ch ) {
    if ( ( int ) ch == -1 ) { // -1 means -1 for cin.peek
      return true ;
    } // if()
    
    return false ;
  } // IsEOF()
  
  void PrintWhite( int num ) {
    for ( int i = 0 ; i < num ; i ++ ) {
      cout << " " ;
    } // for()
  } // PrintWhite()
  
  void PrettyPrintAtom( Node* r ) {
    if ( r -> type == SPECIAL ) {
      if ( r -> lex == "nil" || r -> lex == "#f" ) {
        cout << "nil" << endl ;
      } // if()
      else {
        cout << "#t" << endl ;
      } // else()
    } // if()
    else if ( IsINT( r -> lex ) ) {
      cout << GetValueOfIntStr( r -> lex ) << endl ;
    } // else if()
    else if ( IsFLOAT( r -> lex ) ) {
      cout << fixed << setprecision( 3 ) << GetValueOfFloatStr( r -> lex ) << endl ;
    } // else if()
    else if ( IsStr( r -> lex ) ) {
      PrintStr( r -> lex ) ;
    } // else if()
    else cout << r -> lex << endl ;
  } // PrettyPrintAtom()
  
  void PrettyPrintSExp( Node* r, Direction dir, int level, bool inNewLine ) {
    
    int curLevel = level ;
    
    if ( r == NULL ) {
      return ;
    } // if()
    
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
          if ( inNewLine ) {
            PrintWhite( curLevel ) ;
          } // if()
          
          cout << "(" << " " ;
          PrettyPrintAtom( r -> left ) ;
        } // if()
        else {
          if ( inNewLine ) {
            PrintWhite( curLevel ) ;
          } // if()
          
          cout << "(" << " " ;
          curLevel += 2 ; // A new group, level up
          PrettyPrintSExp( r -> left, LEFT, curLevel, false ) ;
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
          PrettyPrintSExp( r -> left, LEFT, curLevel, inNewLine ) ;
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
    else {
      PrettyPrintSExp( r, LEFT, 0, false ) ;
    } // else()
  } // PrettyPrint()
  
} ;

GlobalFunction g ;

// --------------------- Error Definition Proj.1 (start) ---------------------

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
    // int tmpGLine = gLine ;
    g.Reset() ;
    // gLine = tmpGLine ;
    
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
    + " Column " + g.IntToStr( mCol + ( int ) mStr.length() - 1 ) + " is >>" + mStr + "<<" ;
    g.SkipLine() ;
    // int tmpGLine = gLine ;
    g.Reset() ;
    // gLine = tmpGLine ;
    
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
    // int tmpGLine = gLine ;
    g.Reset() ;
    // gLine = tmpGLine ;
    
    return mesg ;
  } // Err_mesg()
} ; // NoClosingQuoteException

class EOFException {
public:
  string Err_mesg() {
    string mesg = "" ;
    mesg = "ERROR (no more input) : END-OF-FILE encountered" ;
    // int tmpGLine = gLine ;
    g.Reset() ;
    // gLine = tmpGLine ;
    gOriginalList.Clear() ;
    
    return mesg ;
  } // Err_mesg()
} ; // EOFException

// --------------------- Error Definition Proj.1 (end) ---------------------

class LexicalAnalyzer {
  
private:
  
  bool IsSeparator( char ch ) {
    if ( ch == '(' || ch == ')' || ch == '\'' || ch == '\"' || ch == ';' ) {
      return true ;
    } // if()
    
    return false ;
  } // IsSeparator()
  
  bool IsReturnLine( char ch ) {
    if ( ch == '\n' ) {
      return true ;
    } // if()
    
    return false ;
  } // IsReturnLine()
  
  bool IsWhiteSpace( char ch ) {
    // Because Linux has '\r' character, I added '\r' as one circumstance
    if ( ch == ' ' || ch == '\t' || ch == '\r' || IsReturnLine( ch ) ) {
      return true ;
    } // if()
    
    return false ;
  } // IsWhiteSpace()
  
  // Purpose: not only call the func. cin.get(), but also increase the column or line
  char GetChar() {
    char ch = '\0' ;
    ch = cin.get() ;
    if ( g.IsEOF( ch ) ) {
      gIsEOF = true ;
      throw new EOFException() ;
    } // if()
    
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
    
    if ( ch_peek == '\"' && fullStr.length() == 1 ) {
      ch_get = cin.get() ;
      fullStr += ch_get ;
      return fullStr ;
    } // if()
    // because we need to get a string, keep reading the input until
    // encounter the next '\"' or return-line
    
    try {
      
      while ( keepRead && !IsReturnLine( ch_peek ) && !g.IsEOF( ch_peek ) ) {
        ch_get = cin.get() ;
        if ( g.IsEOF( ch_peek ) ) {
          throw new EOFException() ;
        } // if()
        
        fullStr += ch_get ;
        ch_peek = cin.peek() ;
        
        if ( ch_peek == '\"' && ch_get != '\\' )  { // >"< stands alone
          keepRead = false ;
        } // if()
      } // while()
      
    } catch ( EOFException* e ) {
      gColumn += ( int ) fullStr.length() - 1 ;
      throw new NoClosingQuoteException( gLine, gColumn + 1 ) ;
    } // catch()
    
    if ( ch_peek == '\"' ) {  // a complete string with a correct syntax
      ch_get = cin.get() ;
      fullStr += ch_get ;
    } // if()
    else { // miss the ending quote
      gColumn += ( int ) fullStr.length() - 1 ;
      throw new NoClosingQuoteException( gLine, gColumn + 1 ) ;
    } // else()
    
    gColumn += ( int ) fullStr.length() - 1 ;
    
    return fullStr ;
  } // GetFullStr()
  
  Token LexToToken( string lex ) {
    Token token ;
    token.str = "" ;
    token.line = 0 ;
    token.column = 0 ;
    if ( lex != "." && g.IsFLOAT( lex ) ) {
      // assert: float with a original format
      // now  can start trandfer the float into the format which (int).(3 chars)
      token.str = g.FormatFloat( lex ) ;
    } // if()
    else {
      token.str = lex ;
    } // else()
    
    token.line = gLine ;
    token.column = gColumn - ( int ) lex.length() + 1 ;
    token.type = g.GetTokenType( lex ) ;
    
    return token ;
  } // LexToToken()
  
  char SkipWhiteSpace( char ch_get, string &readBuffer ) {
    while ( IsWhiteSpace( ch_get ) || IsReturnLine( ch_get ) ) {
      readBuffer += ch_get ;
      
      if ( IsReturnLine( ch_get ) && gJustFinishAExp ) {
        gJustFinishAExp = false ;
      } // if()
      
      int tmpCinValue = cin.get() ; // take off the return-line and without increase the line
      ch_get = ( char ) tmpCinValue ;
      
    } // while()
    
    if ( ch_get != ';' ) {
      readBuffer += ch_get ;
    } // if()
    
    return ch_get ;
  } // SkipWhiteSpace()
  
  char GetNextNonWhiteChar() {
    char ch_get = GetChar() ;
    
    while ( IsWhiteSpace( ch_get )  ) {
      ch_get = GetChar() ;
    } // while()
    
    return ch_get ;
  } // GetNextNonWhiteChar()
  
public:
  
  string PeekToken() {
    string tokenStrWeGet = "" ;
    
    if ( gPeekToken == "" ) {
      char ch = '\0' ;
      
      // peek whether the next char is in input
      ch = cin.peek() ;
      /*
      if ( g.IsEOF( ch ) ) { // -1 means -1 for cin.peek
        gIsEOF = true ;
        throw new EOFException() ;
      } // if()
      */
      
      // before get a actual char, we need to skip all the white-spaces first
      while ( IsWhiteSpace( ch ) ) {
        ch = cin.get() ;  // take away this white-space
        gColumn ++ ;
        
        if ( IsReturnLine( ch ) ) {
          if ( gJustFinishAExp ) {
            gLine = 1 ;
            gJustFinishAExp = false ;
          } // if()
          else {
            gLine ++ ;
          } // else()
          
          gColumn = 0 ;
          gJustFinishAExp = false ;
        } // if()
        
        ch = cin.peek() ;
        /*
        if ( g.IsEOF( ch ) ) { // -1 means -1 for cin.peek
          gIsEOF = true ;
          throw new EOFException() ;
        } // if()
        */
      } // while()
      
      if ( ch == ';' ) {
        g.SkipLine() ;
        // gLine ++ ; // doesn't count the comment line
        return PeekToken() ;
      } // if()
      
      // assert: finally get a char which is not a white-space, now can start to construct a token
      ch = GetChar() ;  // since this char is not a white-space, we can get it from the input
      tokenStrWeGet += ch ;  // directly add the first non-white-space char into the token string
      
      // if this char is already a separator then STOP reading, or keep getting the next char
      if ( !IsSeparator( ch ) && ch != '\"' ) {  // 'ch' here is the first char overall
        ch = cin.peek() ;
        /*
        if ( g.IsEOF( ch ) ) { // -1 means -1 for cin.peek
          gIsEOF = true ;
          throw new EOFException() ;
        } // if()
        */
        
        // check whether EOF because we may encounter EOF while making a peek token
        while ( !IsSeparator( ch ) && !IsWhiteSpace( ch ) && !g.IsEOF( ch ) ) {
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
    else ;
    
    gJustFinishAExp = false ;
    
    return gPeekToken ;
    
  } // PeekToken()
  
  Token GetToken() {
    
    if ( gPeekToken == "" ) PeekToken() ;
    else ; // PeekToken is already peeked before
    
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
    if ( startToken.type == LPAREN && g.GetTokenType( mLa.PeekToken() ) == RPAREN ) {
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
        
        while ( ( g.GetTokenType( mLa.PeekToken() ) == LPAREN
                  || IsATOM( mLa.PeekToken() )
                  || g.GetTokenType( mLa.PeekToken() ) == QUOTE )
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
          if ( g.GetTokenType( mLa.PeekToken() ) == DOT ) {
            token = mLa.GetToken() ; // must be DOT
            // must be the start of the next S-exp according to the grammer
            token = mLa.GetToken() ;
            
            hasOneSExpCorrect = CheckSExp( token ) ;
            if ( !hasOneSExpCorrect ) {
              throw new MissingAtomOrLeftParException( gLine, gColumn, gPeekToken ) ;
              return false ;
            } // if()
            else {
              if ( g.GetTokenType( mLa.PeekToken() ) == RPAREN ) {
                token = mLa.GetToken() ; // must be >)<
                
                return true ;
              } // if()
              else {
                throw new MissingRightParException( gLine, gColumn, gPeekToken ) ;
                return false ;
              } // else()
            } // else()
            
          } // if()
          else if ( g.GetTokenType( mLa.PeekToken() ) == RPAREN ) {
            token = mLa.GetToken() ; // must be >)<
            
            return true ;
          } // else if()
        } // else()
      } // if()
      else {
        throw new MissingAtomOrLeftParException( gLine, gColumn, token.str ) ;
      } // else()
      
      return false ;
      
    } // else if()
    
    throw new MissingAtomOrLeftParException( gLine, gColumn, startToken.str ) ;
    
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
    TokenType type = g.GetTokenType( str ) ;
    if ( type == SYMBOL || type == INT || type == FLOAT || type == STRING || type == NIL || type == T ) {
      return true ;
    } // if()
    
    return false ;
  } // IsATOM()
  
} ;

class Tree {
  
private:
  
  Node *mRoot ;
  SingleList mCopyList ;
  
  LexicalAnalyzer mLa ;
  SyntaxAnalyzer mS ;
  
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
      nilNode -> isAddByMe = false ;
      
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
        nilNode -> isAddByMe = false ;
        
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
      nilNode -> isAddByMe = false ;
      
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
  
  void TranslateQuote() { // process mCopyList
    Node_Linear* leftPar = new Node_Linear ;
    Node_Linear* quote = new Node_Linear ;
    
    // make a left parathesis node
    leftPar -> token.str = "(" ;
    leftPar -> token.line = -1 ;
    leftPar -> token.column = -1 ;
    leftPar -> token.type = LPAREN ;
    leftPar -> next = NULL ;
    leftPar -> prev = NULL ;
    
    // make a quote node
    quote -> token.str = "quote" ;
    quote -> token.line = -1 ;
    quote -> token.column = -1 ;
    quote -> token.type = QUOTE ;
    quote -> next = NULL ;
    quote -> prev = NULL ;
    
    // from back to front
    for ( Node_Linear* walk = mCopyList.mTail ; walk != NULL ; walk = walk -> prev ) {
      if ( walk -> token.str == "'" ) { // should make the transfer
        if ( walk -> next -> token.type == LPAREN ) {
          Node_Linear* corRightPar = FindCorrespondPar( walk -> next ) ;
          walk -> token.str = "quote" ;
          mCopyList.InsertNode( walk -> prev, LPAREN ) ;
          mCopyList.InsertNode( corRightPar, RPAREN ) ;
        } // if()
        else { // is a aimple symbol
          // mCopyList.InsertNode( mCopyList.mRoot , QUOTE ) ;
          walk -> token.str = "quote" ;
          mCopyList.InsertNode( walk -> prev, LPAREN ) ;
          mCopyList.InsertNode( walk -> next, RPAREN ) ;
        } // else()
      } // if()
    } // for()
  } // TranslateQuote()
  
public:
  Tree( SingleList list ) {
    mRoot = NULL ;
    mCopyList = list ;
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
      atomNode -> isAddByMe = false ;
      
      atomNode -> lex = leftPointer -> next -> token.str ;
      if ( leftPointer -> next -> token.type == NIL || leftPointer -> next -> token.type == T ) {
        atomNode -> type = SPECIAL ;
        
        if ( leftPointer -> next -> isAddByMe ) {
          atomNode -> isAddByMe = true ;
        } // if()
        else ;
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
      atomNode -> isAddByMe = false ;
      
      atomNode -> lex = leftPointer -> token.str ;
      if ( leftPointer -> token.type == NIL || leftPointer -> token.type == T ) {
        atomNode -> type = SPECIAL ;
        
        if ( leftPointer -> isAddByMe ) {
          atomNode -> isAddByMe = true ;
        } // if()
        else ;
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
      cons -> isAddByMe = false ;
      
      cons -> type = CONS ;
      cons -> left = leftSubTree ;
      cons -> left -> parent = cons ;
      cons -> right = rightSubTree ;
      cons -> right -> parent = cons ;
      
      return cons ;
    } // else()
    
    return NULL ;
  } // Build()
  
  void BuildTree() {
    TranslateQuote() ;
    
    // Substitude () with nil and put on the ( )
    if ( mS.IsATOM( mCopyList.mRoot -> token ) ) {
      Node* leaf = new Node ;
      leaf -> lex = "" ;
      leaf -> left = NULL ;
      leaf -> right = NULL ;
      leaf -> parent = NULL ;
      leaf -> type = EMPTY ;
      leaf -> isAddByMe = false ;
      
      leaf -> lex = mCopyList.mRoot -> token.str ;
      if ( mCopyList.mRoot -> token.type == NIL || mCopyList.mRoot -> token.type == T ) {
        leaf -> type = SPECIAL ;
      } // if()
      else {
        leaf -> type = ATOM ;
      } // else()
      
      leaf -> left = NULL ;
      leaf -> right = NULL ;
      leaf -> parent = NULL ;
      mRoot = leaf ;
      
    } // if((
    else {
      if ( mCopyList.mRoot -> token.type == LPAREN && mCopyList.mRoot
          -> next -> token.str == "exit" && mCopyList.mRoot -> next -> next -> token.type == RPAREN ) {
        gIsEOF = true ;
        return ;
      } // if()
      
      Translate( mCopyList.mRoot, mCopyList.mTail ) ;
      // mCopyList.PrintForward() ;
      
      mRoot = Build( mCopyList.mRoot, mCopyList.mTail ) ;
      
      /*
       if ( mRoot -> type == CONS && mRoot -> left -> lex == "exit"
       && mRoot -> right -> lex == "nil" ) {
       gIsEOF = true ;
       } // if()
      */
      // gOriginalList.Print() ;
      // mCopyList.PrintForward() ;
      // mCopyList.PrintBackforward() ;
    } // else()
  } // BuildTree()
  
  Node* GetRoot() {
    return mRoot ;
  } // GetRoot()
  
};

// --------------------- Error Definition Proj.2 (start) ---------------------

class NonListException {
private:
  Node* mErrNode ;
public:
  
  string Err_mesg() {
    string mesg = "ERROR (non-list) : " ;
    return mesg ;
  } // Err_mesg()
  
  Node* Err_node() {
    return mErrNode ;
  } // Err_node()
} ; // NonListException

class IncorrectNumberArgumentException {
private:
  string mLex ;
public:
  IncorrectNumberArgumentException( string funcName ) {
    mLex = funcName ;
  } // IncorrectNumberArgumentException()
  
  string Err_mesg() {
    string mesg = "ERROR (incorrect number of arguments) : " + mLex ;
    return mesg ;
  } // Err_mesg()
} ; // IncorrectNumberArgumentException

class IncorrectArgumentTypeException {
private:
  string mFuncName ;
  
public:
  IncorrectArgumentTypeException( string errFuncName ) {
    mFuncName = errFuncName ;
  } // IncorrectArgumentTypeException()
  
  string Err_mesg() {
    string mesg = "ERROR (" + mFuncName + " with incorrect argument type) : " ;
    return mesg ;
  } // Err_mesg()
  
} ; // IncorrectArgumentTypeException

class ApplyNonFunctionException {
private:
  string mLex ;
  
public:
  
  string Err_mesg() {
    string mesg = "ERROR (attempt to apply non-function) : " ;
    return mesg ;
  } // Err_mesg()
} ; // ApplyNonFunctionException

class NoReturnValueException {
public:
  string Err_mesg() {
    string mesg = "ERROR (no return value) : " ;
    return mesg ;
  } // Err_mesg()
  
} ; // NoReturnValueException

class UnboundValueException {
private:
  string mLex ;
  
public:
  UnboundValueException( string errLex ) {
    mLex = errLex ;
  } // UnboundValueException()
  
  string Err_mesg() {
    string mesg = "ERROR (unbound symbol) : " + mLex ;
    return mesg ;
  } // Err_mesg()
} ; // UnboundValueException

class DivideByZeroException {
public:
  string Err_mesg() {
    string mesg = "ERROR (division by zero) : /" ;
    return mesg ;
  } // Err_mesg()
} ; // DivideByZeroException

class DefineFormatException {
public:
  string Err_mesg() {
    string mesg = "ERROR (DEFINE format) : " ;
    return mesg ;
  } // Err_mesg()
} ; // DefineFormatException

class CondFormatException {
public:
  string Err_mesg() {
    string mesg = "ERROR (COND format) : " ;
    return mesg ;
  } // Err_mesg()
} ; // CondFormatException

class LevelException {
private:
  string mLex ;
public:
  LevelException( string funcName ) {
    mLex = funcName ;
  } // LevelException()
  
  string Err_mesg() {
    string mesg = "ERROR (level of " + mLex + ")" ;
    return mesg ;
  } // Err_mesg()
} ; // CleanLevelException()

// --------------------- Error Definition Proj.2 (end) ---------------------
// --------------------- Error Definition Proj.3 (start) ---------------------
/*
class LetFormatException {
public:
  string Err_mesg() {
    string mesg = "ERROR (let format)" ;
    return mesg ;
  } // Err_mesg()
} ; // LetFormatException

class LambdaFormatException {
public:
  string Err_mesg() {
    string mesg = "ERROR (lambda format)" ;
    return mesg ;
  } // Err_mesg()
} ; // LambdaFormatException
*/

class DefineFormatException2 {
public:
  string Err_mesg() {
    string mesg = "ERROR (define format)" ;
    return mesg ;
  } // Err_mesg()
} ; // DefineFormatException

class FormatException {
private:
  string mLex ;
public:
  FormatException( string funcName ) {
    mLex = funcName ;
  } // FormatException()
  
  string Err_mesg() {
    string mesg = "ERROR (" + mLex + " format) : " ;
    return mesg ;
  } // Err_mesg()
} ; // DefineFormatException

class NonReturnAssignedException {
public:
  string Err_mesg() {
    string mesg = "ERROR (no return value) : " ;
    return mesg ;
  } // Err_mesg()
} ; // NonReturnAssignedException

class ParamNotBoundException {
public:
  string Err_mesg() {
    string mesg = "ERROR (unbound parameter) : " ;
    return mesg ;
  } // Err_mesg()
} ; // ParamNotBoundException

class TestCondNotBoundException {
public:
  string Err_mesg() {
    string mesg = "ERROR (unbound test-condition) : " ;
    return mesg ;
  } // Err_mesg()
} ; // TestCondNotBoundException

class CondNotBoundException {
public:
  string Err_mesg() {
    string mesg = "ERROR (unbound condition) : " ;
    return mesg ;
  } // Err_mesg()
} ; // CondNotBoundException

class AssignedNotBoundException {
public:
  string Err_mesg() {
    string mesg = "ERROR (no return value) : " ;
    return mesg ;
  } // Err_mesg()
} ; // AssignedNotBoundException

class TopLevelNoReturnException {
public:
  string Err_mesg() {
    string mesg = "ERROR (no return value) : " ;
    return mesg ;
  } // Err_mesg()
} ; // TopLevelNoReturnException
// --------------------- Error Definition Proj.3 (end) ---------------------

struct Symbol {
  string name ;
  Node* tree ;
} ; // SymbolInfo

struct LocalSymbol {
  string name ;
  int level ;
  Node* tree ;
} ; // LocalSymbol

struct Function {
  string name ;
  int argNum ;
  vector<string> argList ;
  Node* tree ;
} ; // Function

struct StackArea {
  vector<string> currentArea ;
  StackArea* next ;
  StackArea* prev ;
} ; // StackArea

class CallStack { // to implement a callstack similar to the actual call stack
private:
  vector<string> mCurrentVar ; // record the recent local variable (in smae level)
  StackArea* mStart ;
  StackArea* mEnd ;
  vector<LocalSymbol> mCallStack ; // the first element is the lastest one
  
public:
  CallStack() {
    mStart = NULL ;
    mEnd = NULL ;
  } // CallStack()
  
  void AddNewSymbolNameToStackArea() {
    if ( mStart == NULL ) {
      mStart = new StackArea ;
      mStart -> next = NULL ;
      mStart -> prev = NULL ;
      
      if ( mCurrentVar.size() != 0 ) {
        mStart -> currentArea.assign( mCurrentVar.begin(), mCurrentVar.end() ) ;
      } // if()
      else ; // the current stack area is empty
      
      mEnd = mStart ;
    } // if()
    else { // insert the new names
      mEnd -> next = new StackArea ;
      mEnd -> next -> next = NULL ;
      mEnd -> next -> prev = mEnd ;
      mEnd = mEnd -> next ;
      if ( mCurrentVar.size() != 0 ) {
        mEnd -> currentArea.assign( mCurrentVar.begin(), mCurrentVar.end() ) ;
      } // if()
      else ;
    } // else()
  } // AddNewSymbolNameToStackArea()
  
  void RestoreLocalVar( vector<string> paramList ) {
    mCurrentVar.assign( paramList.begin(), paramList.end() ) ;
  } // RestoreLocalVar()
  
  void DeleteLastStackArea() {
    if ( mEnd == mStart ) {
      if ( mEnd == NULL ) {
        return;
      } // if()
      else {
        delete mStart ;
        mStart = NULL ;
        mEnd = NULL ;
      } // else()
    } // if()
    else if ( mEnd -> prev != NULL ) {
      if ( mEnd -> prev == mStart ) {
        mEnd = mEnd -> prev ;
        delete mStart -> next ;
        mStart -> next = NULL ;
      } // if()
      else {
        mEnd = mEnd -> prev ;
        delete mEnd -> next ;
        mEnd -> next = NULL ;
      } // else()
    } // else if()
  } // DeleteLastStackArea()
  
  void AddLocalVars( vector<string> nameList, vector<Node*> bindList, int level ) {
    if ( nameList.size() != bindList.size() ) {
      cout << "### Error: Local variabl and binding number failure ###" << endl ;
    } // if()
    else {
      for ( int i = 0 ; i < nameList.size() ; i ++ ) {
        AddOneLocalVar( nameList[ i ], bindList[ i ], level ) ;
      } // for()
    } // else()
  } // AddLocalVars()
  
  void AddOneLocalVar( string name, Node* binding, int level ) {
    LocalSymbol newSym ;
    newSym.name = "" ;
    newSym.level = 0 ;
    newSym.tree = NULL ;
    
    newSym.name = name ; // make a new symbol which is a local variable
    newSym.level = level ;
    newSym.tree = binding ;
    
    mCurrentVar.push_back( name ) ;
    mCallStack.push_back( newSym ) ;
  } // AddOneLocalVar()
  
  void GetCleanLocalZone() {
    AddNewSymbolNameToStackArea() ;
    mCurrentVar.clear() ;
  } // GetCleanLocalZone()
  
  void ClearCurrentLocalVar() {
    
    int index = ( int ) mCallStack.size() - 1 ;
    for ( int i = ( int ) mCurrentVar.size() - 1 ; i >= 0 ; i -- ) {
      if ( mCurrentVar[ i ] == mCallStack[ index ].name ) {
        mCallStack.erase( mCallStack.begin() + index ) ;
      } // if()
      
      index -- ;
    } // for()
    
    mCurrentVar.clear() ;
    if ( mEnd != NULL ) {
      mCurrentVar = mEnd -> currentArea ;
      DeleteLastStackArea() ;
    } // if()
  } // ClearCurrentLocalVar()
  
  bool IsLocalVar( string name ) {
    bool isLocal = false ;
    for ( int i = 0 ; i < mCallStack.size() ; i ++ ) {
      if ( name == mCallStack[ i ].name ) {
        isLocal = true ;
      } // if()
    } // for()
    
    return isLocal ;
  } // IsLocalVar()
  
  int GetLocalVarIndex( string varName ) {
    for ( int i = ( int ) mCallStack.size() - 1 ; i >= 0 ; i -- ) {
      if ( varName == mCallStack[ i ].name ) {
        return i ;
      } // if()
    } // for()
    
    return -1 ;
  } // GetLocalVarIndex()
  
  Node* GetLocalVarBinding( string varName ) {
    for ( int i = ( int ) mCallStack.size() - 1 ; i >= 0 ; i -- ) {
      if ( varName == mCallStack[ i ].name ) {
        return mCallStack[ i ].tree ;
      } // if()
    } // for()
    
    return NULL ;
  } // GetLocalVarBinding()
  
  void UpdateVar( int index, Node* newBinding ) {
    mCallStack[ index ].tree = newBinding ;
  } // UpdateVar()
  
  void CleanStack() {
    mCallStack.clear() ;
    mStart = NULL ;
    mEnd = NULL ;
  } // CleanStack()
  
} ; // CallStack

// Purpose: Do the evaluation and store the user definitions
class Evaluator {
private:
  
  struct ReserveWord {
    string name ;
    vector<string> list ;
  } ; // ReserveWord
  
  vector<Symbol> mSymbolTable ;
  vector<Function> mFunctionTable ;
  vector<ReserveWord> mReserveWords ;
  CallStack mCallStack ;
  vector<Function> mUserDefinedFunctionTable ;
  Function mLambdaFunc ; // used to temporary store the lambda function
  vector<Function> mLambdaStack ;
  
  void InitialReserveWord() {
    for ( int i = 0 ; i < gReserveWordNum ; i ++ ) {
      ReserveWord tmpWord ;
      tmpWord.name = gOriginReserveWordList[ i ] ;
      tmpWord.list.clear() ;
      mReserveWords.push_back( tmpWord ) ;
    } // for()
  } // InitialReserveWord()
  
  void ResetReserveWord() {
    for ( int i = 0 ; i < gReserveWordNum ; i ++ ) {
      mReserveWords[ i ].list.clear() ;
    } // for()
  } // ResetReserveWord()
  
  void DeleteTree( Node* root ) {
    if ( root != NULL ) {
      // leaf
      if ( root -> left == NULL && root -> right == NULL ) {
        delete root ;
        root = NULL ;
      } // if()
      else  { // still some subtrees in left or right node
        if ( root -> left != NULL ) {
          return DeleteTree( root -> left ) ;
        } // if()
        
        if ( root -> right != NULL ) {
          return DeleteTree( root -> right ) ;
        } // if()
      } // else()
    } // if()
  } // DeleteTree()
  
  void UpdateGlobalSymbol( string symName, Node* assignedTree, int level ) {
    if ( !mCallStack.IsLocalVar( symName ) ) {
      int symIndex = FindSymbolFromLocalAndGlobal( symName ) ;
      
      mSymbolTable[ symIndex ].name = symName ;
      mSymbolTable[ symIndex ].tree = CopyTree( assignedTree ) ;
    } // if()
    else {
      cout << "### Error: this is a local variable ###" << endl ;
    } // else()
  } // UpdateGlobalSymbol()
  
  Node* CopyNode( Node* node ) {
    Node* newN = g.GetEmptyNode() ;
    newN -> lex = node -> lex ;
    newN -> type = node -> type ;
    newN -> left = node -> left ;
    newN -> right = node -> right ;
    newN -> isAddByMe = node -> isAddByMe ;
    newN -> parent = NULL ;
    
    return newN ;
  } // CopyNode()
  
  Node* CopyTree( Node* oldTree ) {
    if ( oldTree == NULL ) {
      return NULL ;
    } // if()
    
    Node* newLeft = NULL ;
    Node* newRight = NULL ;
    
    if ( oldTree -> left != NULL ) {
      newLeft = CopyTree( oldTree -> left ) ;
    } // if()
    else newLeft = NULL ;
    
    if ( oldTree -> right != NULL ) {
      newRight = CopyTree( oldTree -> right ) ;
    } // if()
    else newRight = NULL ;
    
    Node* newTree = CopyNode( oldTree ) ;
    newTree -> left = newLeft ;
    newTree -> right = newRight ;
    if ( newLeft != NULL ) {
      newLeft -> parent = newTree ;
    } // if()
    else ;
    if ( newRight != NULL ) {
      newLeft -> parent = newTree ;
    } // if()
    else ;
    
    return newTree ;
  } // CopyTree()
  
  void AddSymbol( string symName, Node* assignedTree ) {
    Symbol symbol ;
    symbol.name = "" ;
    symbol.tree = NULL ;
    symbol.name = symName ;
    symbol.tree = assignedTree ; // not only copy the pointer but the content
    
    mSymbolTable.push_back( symbol ) ; // add this new symbol to the table
  } // AddSymbol()
  
  void AddUserDefineFunction( string funcName, int numberOfAug, Node* assignedTree ) {
    Function func ;
    func.name = "" ;
    func.argNum = 0 ;
    func.tree = NULL ;
    
    func.name = funcName ;
    func.argNum = numberOfAug ;
    // func.tree = assignedTree ;
    func.tree = CopyTree( assignedTree ) ;
    
    mUserDefinedFunctionTable.push_back( func ) ;
  } // AddUserDefineFunction()
  
  string GetReserveWordType( string str ) {
    str = GetFuncNameFromFuncValue( str ) ;
    
    for ( int i = 0 ; i < mReserveWords.size() ; i ++ ) {
      if ( str == mReserveWords[ i ].name ) {
        return mReserveWords[ i ].name ;
      } // if()
      
      for ( int j = 0 ; j < mReserveWords[ i ].list.size() ; j ++ ) {
        if ( str == mReserveWords[ i ].list[ j ] ) {
          return mReserveWords[ i ].name ;
        } // if()
      } // for()
    } // for()
    
    return "" ;
  } // GetReserveWordType()
  
  string GetReserveWordType( string str, int &index ) {
    str = GetFuncNameFromFuncValue( str ) ;
    
    for ( int i = 0 ; i < mReserveWords.size() ; i ++ ) {
      if ( str == mReserveWords[ i ].name ) {
        index = i ;
        return mReserveWords[ i ].name ;
      } // if()
      
      for ( int j = 0 ; j < mReserveWords[ i ].list.size() ; j ++ ) {
        if ( str == mReserveWords[ i ].list[ j ] ) {
          index = i ;
          return mReserveWords[ i ].name ;
        } // if()
      } // for()
    } // for()
    
    return "" ;
  } // GetReserveWordType()
  
  bool IsPredicator( string str ) {
    if ( str == "atom?" || str == "pair?" || str == "list?" || str == "null?"
         || str == "integer?" || str == "real?" || str == "number?"
         || str == "string?" || str == "boolean?" || str == "symbol?" ) {
      return true ;
    } // if()
    
    return false ;
  } // IsPredicator()
  
  int FindGlobalSymbol( string str ) {
    int index = -1 ;
    
    // here is the global variable
    for ( int i = 0 ; i < mSymbolTable.size() && index == -1 ; i ++ ) {
      if ( str == mSymbolTable[ i ].name ) {
        index = i ;
      } // if()
    } // for()
    
    return index ;
  } // FindGlobalSymbol()
  
  bool IsGlobalSymbol( string str ) {
    for ( int i = 0 ; i < mSymbolTable.size() ; i ++ ) {
      if ( str == mSymbolTable[ i ].name ) {
        return true ;
      } // if()
    } // for()
    
    return false ;
  } // IsGlobalSymbol()
  
  int FindLocalSymbol( string str ) {
    return mCallStack.GetLocalVarIndex( str ) ;
  } // FindLocalSymbol()
  
  int FindSymbolFromLocalAndGlobal( string str ) {
    
    if ( mCallStack.IsLocalVar( str ) ) { // If this is a local variable, then find in stack
      return FindLocalSymbol( str ) ;
    } // if()
    
    return FindGlobalSymbol( str ) ;
  } // FindSymbolFromLocalAndGlobal()
  
  bool SymbolExist( string str ) {
    if ( mCallStack.IsLocalVar( str ) ) {
      return true ;
    } // if()
    else if ( IsGlobalSymbol( str ) ) {
      return true ;
    } // else if()
    else ; // symbol doesn't exist
    
    return false ;
  } // SymbolExist()
  
  Node* FindSymbolBinding( string str ) {
    Node* bindResult = NULL ;
    // always find in the local variable first
    if ( mCallStack.IsLocalVar( str ) ) {
      bindResult = mCallStack.GetLocalVarBinding( str ) ;
    } // if()
    else { // not a local variable, now try to find in the global area
      int index = FindGlobalSymbol( str ) ;
      if ( index != -1 ) {
        bindResult = mSymbolTable[ index ].tree ;
      } // if()
      else ; // not a local variable, neither a global variable
    } // else()
    
    return bindResult ;
  } // FindSymbolBinding()
  
  int FindDefinedFunc( string str ) {
    int index = -1 ;
    
    for ( int i = 0 ; i < mUserDefinedFunctionTable.size() && index == -1 ; i ++ ) {
      if ( str == mUserDefinedFunctionTable[ i ].name ) {
        index = i ;
      } // if()
    } // for()
    
    return index ;
  } // FindDefinedFunc()
  
  int FindUserDefinedFunc( string str ) {
    int index = -1 ;
    str = GetFuncNameFromFuncValue( str ) ;
    
    for ( int i = 0 ; i < mUserDefinedFunctionTable.size() && index == -1 ; i ++ ) {
      if ( str == mUserDefinedFunctionTable[ i ].name ) {
        index = i ;
      } // if()
    } // for()
    
    return index ;
  } // FindUserDefinedFunc()
  
  bool IsList( Node* origin ) {
    Node* walk = origin ;
    for ( ; walk -> right != NULL ; walk = walk -> right ) { } ;
    
    if ( walk -> type != CONS && walk -> lex == "nil" ) {
      return true ;
    } // if()
    else ;
    
    return false ;
  } // IsList()
  
  /*
  bool IsList( Node* originRoot, Node* root ) {
    // the last node (should be an atom)
    if ( root != NULL && ( root -> type == ATOM || root -> type == SPECIAL ) ) {
      if ( ( root -> lex == "nil" ) || root == originRoot ) {
        return true ;
      } // if()
      
      return false ;
    } // if()
    
    return IsList( originRoot, root -> right ) ;
  } // IsList()
  */
  
  int CountListElement( Node* tree ) {
    if ( tree == NULL ) {
      return -1 ;
    } // if()
    
    return CountListElement( tree -> right ) + 1 ;
  } // CountListElement()
  
  int CountArgument( Node* tree ) {
    if ( tree == NULL ) {
      return -2 ;
    } // if()
    
    return CountArgument( tree -> right ) + 1 ;
  } // CountArgument()
  
  bool IsSymbol( string str ) {
    if ( str != "" && g.GetTokenType( str ) == SYMBOL ) {
      return true ;
    } // if()
    
    return false ;
  } // IsSymbol()
  
  bool IsUserDefinedFunc( string str ) {
    for ( int i = 0 ; i < mUserDefinedFunctionTable.size() ; i ++ ) {
      if ( str == mUserDefinedFunctionTable[ i ].name ) {
        return true ;
      } // if()
    } // for()
    
    return false ;
  } // IsUserDefinedFunc()
  
  // Purpose combined the trees
  Node* EvaluateCONS( Node* inTree, int level ) {
    Node* consNode = NULL ;
    // Step1. check argument num ( cons can only have 2 )
    if ( CountArgument( inTree ) == 2 ) {
      // Step2. check argument type
      // in CONS case, both arguments of CONS should be a tree (list)
      Node* firstArg = inTree -> right -> left ; // assume aug1 is a list
      Node* secondArg = inTree -> right -> right -> left ; // assume aug2 is a list
      
      consNode = new Node ;
      consNode -> lex = "" ;
      consNode -> type = CONS ;
      consNode -> left = NULL ;
      consNode -> right = NULL ;
      consNode -> parent = NULL ;
      consNode -> isAddByMe = false ;
      
      if ( IsList( firstArg ) || firstArg -> type != CONS ) {
        if ( firstArg -> type == ATOM
             && IsSymbol( firstArg -> lex  )
             && FindSymbolFromLocalAndGlobal( firstArg -> lex ) == -1 ) {
          throw new UnboundValueException( firstArg -> lex ) ;
        } // if()
        else {
          Node* leftCons = EvaluateSExp( firstArg, ++level ) ;
          if ( !CheckHasReturnBinding( leftCons, firstArg ) ) {
            throw new ParamNotBoundException() ;
          } // if()
          else {
            consNode -> left = leftCons ;
          } // else()
          
          
          if ( IsList( secondArg ) || secondArg -> type != CONS ) {
            if ( secondArg -> type == ATOM
                 && IsSymbol( secondArg -> lex )
                 && FindSymbolFromLocalAndGlobal( secondArg -> lex ) == -1 ) {
              throw new UnboundValueException( secondArg -> lex ) ;
            } // if()
            else {
              // Step3. If no error create a new Node
              Node* rightCons = EvaluateSExp( secondArg, ++level ) ;
              if ( !CheckHasReturnBinding( rightCons, secondArg ) ) {
                throw new ParamNotBoundException() ;
              } // if()
              else {
                consNode -> right = rightCons ;
              } // else()
            } // else()
          } // if()
          else {
            gErrNode = secondArg ;
            throw new NonListException() ; // curious
          } // else()
        } // else()
      } // if()
      else {
        gErrNode = firstArg ;
        throw new NonListException() ; // curious
      } // else()
    } // if()
    else {
      throw new IncorrectNumberArgumentException( "cons" ) ; // curious here
    } // else()
    
    return consNode ;
  } // EvaluateCONS()
  
  vector<Node*> GetArgumentList( Node* inTree ) {
    vector<Node*> list ;
    for ( Node* walk = inTree -> right ; walk -> right != NULL ; walk = walk -> right ) {
      if ( walk != NULL ) {
        list.push_back( walk -> left ) ;
      } // if()
    } // for()
    
    return list ;
  } // GetArgumentList()
  
  Node* EvaluateLIST( Node* inTree, int level ) {
    Node* result = NULL ;
    vector<Node*> argList ;
    if ( CountArgument( inTree ) > 0 ) {
      // Step1. find out all the arguments and put them in a list
      argList = GetArgumentList( inTree )  ;
      // Step2. start to check the type and mean time replace the symbol
      for ( int i = 0 ; i < argList.size() ; i ++ ) {
        if ( IsList( argList[ i ] ) || argList[ i ] -> type != CONS ) {
          if ( argList[ i ] -> type == ATOM
               && IsSymbol( argList[ i ] -> lex ) ) {
            int symIndex = FindSymbolFromLocalAndGlobal( argList[ i ] -> lex ) ;
            if ( symIndex != -1 ) {
              Node* result = EvaluateSExp( argList[ i ], ++level ) ;
              CheckHasReturnBindingOrThrow( result, argList[ i ] ) ;
              // the result of this element in the list has a binding, keep going
              argList[ i ] = result ;
            } // if()
            else {
              throw new UnboundValueException( argList[ i ] -> lex ) ;
            } // else()
          } // if()
          else { // this is a list, but need to check more detail
            Node* result = EvaluateSExp( argList[ i ], ++level ) ;
            CheckHasReturnBindingOrThrow( result, argList[ i ] ) ;
            // the result of this element in the list has a binding, keep going
            argList[ i ] = result ;
          } // else()
        } // if()
        else {
          gErrNode = argList[ i ] ;
          throw new NonListException() ;
        } // else()
      } // for()
      // Step3. All the arguments are correct, now combined them
      Node* prevNode = NULL ;
      for ( int i = 0 ; i < argList.size() ; i ++ ) {
        Node* node = new Node ;
        node -> lex = "" ;
        node -> type = CONS ;
        node -> parent = NULL ;
        node -> left = NULL ;
        node -> right = NULL ;
        node -> isAddByMe = false ;
        
        node -> left = argList[ i ] ;
        
        if ( i == 0 ) {
          result = node ;
          prevNode = node ;
        } // if()
        
        if ( i == argList.size() - 1 ) {
          prevNode -> right = node ;
          node -> parent = prevNode ;
          
          Node* nilNode = new Node ;
          nilNode -> lex = "nil" ;
          nilNode -> type = SPECIAL ;
          nilNode -> parent = node ;
          nilNode -> left = NULL ;
          nilNode -> right = NULL ;
          nilNode -> isAddByMe = true ;
          node -> right = nilNode ;
          
        } // if()
        else {
          prevNode -> right = node ;
          node -> parent = prevNode ;
          
          prevNode = node ;
        } // else()
      } // for()
      
      return result ;
    } // if()
    else if ( CountArgument( inTree ) == 0 ) { // no element list
      return g.GetNullNode() ;
    } // else if()
    else ;
    
    return NULL ; // it is empty in the argument
  } // EvaluateLIST()
  
  bool IsOriginalReserveWord( string name ) {
    for ( int i = 0 ; i < gReserveWordNum ; i ++ ) {
      if ( name == mReserveWords[ i ].name ) {
        return true ;
      } // if()
    } // for()
    
    return false ;
  } // IsOriginalReserveWord()
  
  void EraseUserDefinedReserveWord( int index, string name ) {
    if ( index >= 0 ) {
      for ( int i = 0 ; i < mReserveWords[ index ].list.size() ; i ++ ) {
        if ( name == mReserveWords[ index ].list[ i ] ) {
          mReserveWords[ index ].list.erase( mReserveWords[ index ].list.begin() + i ) ;
        } // if()
      } // for()
    } // if()
  } // EraseUserDefinedReserveWord()
  
  void AddNewReserveWord( string reserveName, string newName ) {
    for ( int i = 0 ; i < gReserveWordNum ; i ++ ) {
      if ( reserveName == mReserveWords[ i ].name ) {
        mReserveWords[ i ].list.push_back( newName ) ;
        return ;
      } // if()
    } // for()
  } // AddNewReserveWord()
  
  void UpdateUserDefinedFunc( string funcName, Function func ) {
    int funcIndex = FindUserDefinedFunc( funcName ) ;
    func.name = funcName ;
    
    if ( funcIndex != -1 ) { // this function is already exist
      if ( func.tree == NULL ) { // should delete the function with this name
        mUserDefinedFunctionTable.erase( mUserDefinedFunctionTable.begin() + funcIndex ) ;
      } // if()
      else {
        mUserDefinedFunctionTable[ funcIndex ] = func ;
      } // else()
    } // if()
    else { // a new function name
      mUserDefinedFunctionTable.push_back( func ) ;
    } // else()
  } // UpdateUserDefinedFunc()
  
  bool IsALambdaFunc( Node* tree ) {
    if ( tree -> left -> type != CONS ) {
      if ( tree -> left -> lex == "lambda" ) {
        return true ;
      } // if()
      
      return false ;
    } // if()
    
    return IsALambdaFunc( tree -> left ) ;
  } // IsALambdaFunc()
  
  Node* DefineUserFunc( Node* inTree, int level ) {
    Node* funcNameAndArgPart = inTree -> right -> left ;
    Function newFunc ;
    newFunc.name = "" ;
    newFunc.argNum = 0 ;
    newFunc.tree = NULL ;
    Symbol newSymbol ;
    newSymbol.name = "" ;
    newSymbol.tree = g.GetEmptyNode() ;
    
    
    if ( IsList( funcNameAndArgPart ) ) {
      string funcName = inTree -> right -> left -> left -> lex ;
      Node* argList = inTree -> right -> left -> right ;
      Node* procedurePart = inTree -> right -> right ;
      
      // if the function name is not a symbol and the binding has more than one
      if ( !g.IsSymbol( funcName ) || procedurePart == NULL
           || CountListElement( procedurePart ) == 0 ) {
        gErrNode = inTree ;
        throw new DefineFormatException() ;
      } // if()
      
      Function newFunc ;
      newFunc.name = funcName ;
      newFunc.argNum = 0 ;
      newFunc.tree = NULL ;
      
      newFunc.argNum = CountAndCkeckParameters( argList, newFunc.argList ) ;
      newFunc.tree = procedurePart ;
      // newFunc.tree = CopyTree( procedurePart ) ;
      
      // create a global symbol for this function, make it easy to find in the symbol
      Node* tmp = g.GetEmptyNode() ;
      if ( IsALambdaFunc( procedurePart ) ) {
        tmp -> lex = "#<procedure lambda>" ;
      } // if()
      else {
        tmp -> lex = "#<procedure " + funcName + ">" ;
      } // else()
      
      tmp -> type = ATOM ;
      
      if ( FindGlobalSymbol( funcName ) == -1 ) {
        AddSymbol( funcName, tmp ) ;
      } // if()
      else {
        UpdateGlobalSymbol( funcName, tmp, level ) ;
      } // else()
      
      UpdateUserDefinedFunc( funcName, newFunc ) ;
      
      if ( gVerbose ) {
        cout << funcName << " defined" << endl ;
      } // if()
      
    } // if()
    else {
      gErrNode = inTree ;
      throw new DefineFormatException() ;
    } // else()
    
    return NULL ;
  } // DefineUserFunc()
  
  bool IsQuoteExp( Node* tree ) {
    if ( tree -> type == CONS && tree -> left -> lex == "quote" ) {
      return true ;
    } // if()
    
    return false ;
  } // IsQuoteExp()
  
  Node* Define( Node* inTree, int level ) {
    vector<Node*> argList = GetArgumentList( inTree ) ;
    
    if ( level == 1 ) {
      if ( inTree -> right -> left != NULL && inTree -> right -> left -> type == CONS ) {
        return DefineUserFunc( inTree, ++level ) ;
      } // if()
      
      if ( CountArgument( inTree ) == 2 ) {
        Symbol newSymbol ;
        newSymbol.name = "" ;
        newSymbol.tree = NULL ;
        // the first argument should be a symbol
        if ( argList[ 0 ] -> type == ATOM ) {
          int reserveIndex = -1 ;
          string reserveName = GetReserveWordType( argList[ 0 ] -> lex, reserveIndex ) ;
          
          if ( reserveName != "" && !IsOriginalReserveWord( argList[ 0 ] -> lex ) ) {
            // may be a user new defined reverse word
            EraseUserDefinedReserveWord( reserveIndex, argList[ 0 ] -> lex ) ;
            reserveName = "" ;
            
          } // if()
          
          if ( reserveName == "" ) {
            if ( g.GetTokenType( argList[ 0 ] -> lex ) == SYMBOL ) {
              
              int symIndex = FindSymbolFromLocalAndGlobal( argList[ 0 ] -> lex ) ;
              newSymbol.name = argList[ 0 ] -> lex ;
              // check the be binded s-exp is correct
              
              Node* bind = NULL ;
              if ( IsQuoteExp( argList[ 1 ] ) ) {
                // define should be the original binding except the quote
                bind = argList[ 1 ] -> right -> left ;
              } // if()
              else {
                // the definition of this symbol, need to check whether the binding exist
                // if not, then this is a non return value error
                bind = EvaluateSExp( argList[ 1 ], ++level ) ;
                CheckHasReturnBindingOrThrow( bind, argList[ 1 ] ) ;
                
              } // else()
              
              if ( symIndex != -1 ) { // this symbol has already exist, update it
                
                if ( FindSymbolFromLocalAndGlobal( argList[ 1 ] -> lex ) == -1 ) {
                  // the reference value is not a symbol
                  if ( mCallStack.IsLocalVar( newSymbol.name ) ) {
                    mCallStack.UpdateVar( symIndex, bind ) ;
                  } // if()
                  else { // this new symbol should be a global symbol
                    // mSymbolTable[ symIndex ].tree = bind ; // copy
                    mSymbolTable[ symIndex ].tree = CopyTree( bind ) ;
                    
                    if ( bind -> type == ATOM && bind -> lex == "#<procedure lambda>" ) {
                      UpdateUserDefinedFunc( newSymbol.name, mLambdaFunc ) ;
                    } // if()
                    else if ( IsUserDefinedFunc( newSymbol.name ) ) {
                      // this symbol name has already used to define a function
                      // but now this symbol is used to a new binding which is not a function
                      // so need to remove this symbol name from the UserDefinedFuncTable
                      Function empty ;
                      empty.name = "" ;
                      empty.argNum = 0 ;
                      empty.tree = NULL ;
                      
                      UpdateUserDefinedFunc( newSymbol.name, empty ) ;
                    } // else if()
                    
                    string reserveName =
                    GetReserveWordType( GetFuncNameFromFuncValue( bind -> lex ) ) ;
                    if ( reserveName != "" ) {
                      AddNewReserveWord( reserveName, newSymbol.name ) ;
                    } // if()
                  } // else()
                } // if()
                else {
                  string reserveName = GetReserveWordType( argList[ 1 ] -> lex ) ;
                  if ( reserveName != "" ) {
                    AddNewReserveWord( reserveName, argList[ 0 ] -> lex ) ;
                  } // if()
                  
                  UpdateGlobalSymbol( argList[ 0 ] -> lex, bind, level ) ;
                } // else()
              } // if()
              else {
                if ( FindSymbolFromLocalAndGlobal( argList[ 1 ] -> lex ) == -1 ) {
                  // the reference value is not a symbol
                  
                  // this been assigned S-exp is in the input
                  // and haven't evaluated yet
                  newSymbol.tree = bind ;
                } // if()
                else {
                  int symIndex = FindSymbolFromLocalAndGlobal( argList[ 1 ] -> lex ) ;
                  if ( mCallStack.IsLocalVar( argList[ 1 ] -> lex ) ) {
                    // newSymbol.tree = mCallStack.GetLocalVarBinding( argList[ 1 ] -> lex ) ;
                    newSymbol.tree = CopyTree( mCallStack.GetLocalVarBinding( argList[ 1 ] -> lex ) ) ;
                  } // if()
                  else {
                    newSymbol.tree = mSymbolTable[ symIndex ].tree ;
                  } // else()
                } // else()
                
                if ( newSymbol.tree -> type == ATOM && newSymbol.tree -> lex != "#<procedure lambda>" ) {
                  string reserveName1 =
                  GetReserveWordType( GetFuncNameFromFuncValue( newSymbol.tree -> lex ) ) ;
                  string reserveName2 = GetReserveWordType( newSymbol.tree -> lex ) ;
                  
                  if ( reserveName1 != "" ) {
                    // this is a special case, define your own reserve word
                    // add this to the reserveWordList
                    AddNewReserveWord( reserveName1, newSymbol.name ) ;
                  } // if()
                  else if ( reserveName2 != "" ) {
                    AddNewReserveWord( reserveName2, newSymbol.name ) ;
                  } // else if()
                } // if()
                
                if ( newSymbol.tree -> type != CONS
                     && newSymbol.tree -> lex == "#<procedure lambda>" ) {
                  mLambdaFunc.name = newSymbol.name ;
                  UpdateUserDefinedFunc( mLambdaFunc.name, mLambdaFunc ) ;
                } // if()
                
                AddSymbol( newSymbol.name, newSymbol.tree ) ;
              } // else()
              
            } // if()
            else {
              gErrNode = inTree ;
              throw new DefineFormatException() ;
            } // else()
          } // if()
          else { // user try to re-define the reserve word
            gErrNode = inTree ;
            throw new DefineFormatException() ; // curious
          } // else()
          
          if ( gVerbose ) {
            cout << newSymbol.name << " defined" << endl ;
          } // if()
          
        } // if()
        else { //
          gErrNode = inTree ;
          throw new DefineFormatException() ; // curious
        } // else()
      } // if()
      else {
        gErrNode = inTree ;
        throw new DefineFormatException() ; // curious
      } // else()
    } // if()
    else {
      throw new LevelException( "DEFINE" ) ; // curious here
    } // else()
    
    return NULL ;
  } // Define()
  
  Node* AccessList( string funcName, Node* inTree, int level ) {
    if ( CountArgument( inTree ) == 1 ) {
      Node* targetTree = EvaluateSExp( inTree -> right -> left, ++level ) ;
      
      if ( targetTree != NULL ) {
        if ( targetTree -> type == ATOM || targetTree -> type == SPECIAL ) {
          gErrNode = targetTree ;
          throw new IncorrectArgumentTypeException( funcName ) ;
        } // if()
        else {
          if ( funcName == "car" ) {
            return targetTree -> left ;
          } // if()
          else if ( funcName == "cdr" ) {
            return targetTree -> right ;
          } // else if()
        } // else()
      } // if()
      else {
        gErrNode = inTree -> right -> left ;
        throw new IncorrectArgumentTypeException( funcName ) ;
      } // else()
    } // if()
    else {
      throw new IncorrectNumberArgumentException( funcName ) ; // curious here
    } // else()
    
    return NULL ;
  } // AccessList()
  
  Node* PrimitivePredecates( string func, Node* inTree, int level ) {
    bool ans = false ;
    Node* ansNode = new Node ;
    ansNode -> lex = "#t" ;
    ansNode -> type = SPECIAL ;
    ansNode -> parent = NULL ;
    ansNode -> left = NULL ;
    ansNode -> right = NULL ;
    ansNode -> isAddByMe = false ;
    // can only have ONE argement
    if ( CountArgument( inTree ) == 1 ) {
      Node* target = EvaluateSExp( inTree -> right -> left, ++level ) ;
      
      if ( target != NULL ) {
        if ( func == "atom?" ) {
          if ( target -> type == ATOM || target -> type == SPECIAL ) {
            ans = true ;
          } // if()
        } // if()
        else if ( func == "pair?" ) {
          if ( target -> type == CONS ) {
            ans = true ;
          } // if()
        } // else if()
        else if ( func == "list?" ) {
          if ( target -> type == CONS && IsList( target ) ) {
            ans = true ;
          } // if()
        } // else if()
        else if ( func == "null?" ) {
          if ( target -> lex == "nil" || target -> lex == "#f" ) {
            ans = true ;
          } // if()
        } // else if()
        else if ( func == "integer?" ) {
          if ( target -> type == ATOM && g.IsINT( target -> lex ) ) {
            ans = true ;
          } // if()
        } // else if()
        else if ( func == "real?" || func == "number?" ) {
          if ( target -> type == ATOM && ( g.IsINT( target -> lex ) || g.IsFLOAT( target -> lex ) ) ) {
            ans = true ;
          } // if()
        } // else if()
        else if ( func == "string?" ) {
          if ( target -> type == ATOM && g.IsStr( target -> lex ) ) {
            ans = true ;
          } // if()
        } // else if()
        else if ( func == "boolean?" ) {
          if ( target -> type == SPECIAL ) {
            ans = true ;
          } // if()
        } // else if()
        else if ( func == "symbol?" ) {
          if ( target -> type == ATOM && g.GetTokenType( target -> lex ) == SYMBOL ) {
            ans = true ;
          } // if()
        } // else if()
      } // if()
      else {
        gErrNode = inTree -> right -> left ;
        throw new IncorrectArgumentTypeException( func ) ;
      } // else()
      
      if ( ans ) {
        ansNode -> lex = "#t" ;
      } // if()
      else {
        ansNode -> lex = "nil" ;
      } // else()
      
      return ansNode ;
    } // if()
    else {
      throw new IncorrectNumberArgumentException( func ) ;
    } // else()
    
    return NULL ;
  } // PrimitivePredecates()
  
  Node* CleanEnvironment() {
    // mSymbolTable.clear() ;
    mFunctionTable.clear() ;
    mUserDefinedFunctionTable.clear() ;
    CleanWholeStack() ;
    
    ResetSymbolTable() ;
    ResetReserveWord() ;
    
    if ( gVerbose ) {
      cout << "environment cleaned" << endl ;
    } // if()
    
    return NULL ;
  } // CleanEnvironment()
  
  bool IsMathOperator( string str ) {
    if ( str == "+" || str == "-" || str == "*" || str == "/" ) {
      return true ;
    } // if()
    
    return false ;
  } // IsMathOperator()
  
  bool IsComparison( string str ) {
    if ( str == ">" || str == "<" || str == ">=" || str == "<=" || str == "=" ) {
      return true ;
    } // if()
    
    return false ;
  } // IsComparison()
  
  bool IsStringOperator( string str ) {
    if ( str == "string-append" || str == "string>?" || str == "string<?" || str == "string=?" ) {
      return true ;
    } // if()
    
    return false ;
  } // IsStringOperator()
  
  bool IsCondOperator( string str ) {
    if ( str == "not" || str == "and" || str == "or" ) {
      return true ;
    } // if()
    
    return false ;
  } // IsCondOperator()
  
  bool IsBasicOperation( string str ) {
    if ( IsMathOperator( str ) || IsComparison( str )
         || IsStringOperator( str ) || IsCondOperator( str ) ) {
      return true ;
    } // if()
    
    return false ;
  } // IsBasicOperation()
  
  Node* ProcessMath( string funcName, vector<Node*> argList, int level ) {
    Node* ansNode = new Node ;
    ansNode -> lex = "" ;
    ansNode -> type = ATOM ;
    ansNode -> parent = NULL ;
    ansNode -> left = NULL ;
    ansNode -> right = NULL ;
    ansNode -> isAddByMe = false ;
    
    // check whether all the arguments are numbers
    for ( int i = 0 ; i < argList.size() ; i ++ ) {
      Node* currentAug = EvaluateSExp( argList[ i ], ++level ) ;
      if ( currentAug != NULL && currentAug -> type == ATOM
           && ( g.IsINT( currentAug -> lex )
                || g.IsFLOAT( currentAug -> lex ) ) ) {
        argList[ i ] = currentAug ;
      } // if()
      else if ( currentAug == NULL ) { // a non number atom exist
        gErrNode = argList[ i ] ;
        throw new ParamNotBoundException() ;
      } // else if()
      else {
        gErrNode = currentAug ;
        throw new IncorrectArgumentTypeException( funcName ) ;
      } // else()
    } // for()
    
    double ans = 0.0 ; // in case the result bigger than the range of INT
    bool hasFloatExist = false ;
    
    if ( g.IsINT( argList[ 0 ] -> lex ) ) {
      ans = g.GetValueOfIntStr( argList[ 0 ] -> lex ) ;
    } // if()
    else if ( g.IsFLOAT( argList[ 0 ] -> lex ) ) {
      hasFloatExist = true ;
      ans = g.GetValueOfFloatStr( argList[ 0 ] -> lex ) ;
    } // else if()
    
    for ( int i = 1 ; i < argList.size() ; i ++ ) {
      if ( g.IsINT( argList[ i ] -> lex ) ) {
        if ( funcName == "+" ) {
          ans += g.GetValueOfIntStr( argList[ i ] -> lex ) ;
        } // if()
        else if ( funcName == "-" ) {
          ans -= g.GetValueOfIntStr( argList[ i ] -> lex ) ;
        } // else if()
        else if ( funcName == "*" ) {
          ans *= g.GetValueOfIntStr( argList[ i ] -> lex ) ;
        } // else if()
        else if ( funcName == "/" ) {
          if ( g.GetValueOfIntStr( argList[ i ] -> lex ) != 0 ) {
            ans /= g.GetValueOfIntStr( argList[ i ] -> lex ) ;
          } // if()
          else {
            throw new DivideByZeroException() ;
          } // else()
        } // else if()
      } // if()
      else if ( g.IsFLOAT( argList[ i ] -> lex ) ) {
        hasFloatExist = true ;
        if ( funcName == "+" ) {
          ans += g.GetValueOfFloatStr( argList[ i ] -> lex ) ;
        } // if()
        else if ( funcName == "-" ) {
          ans -= g.GetValueOfFloatStr( argList[ i ] -> lex ) ;
        } // else if()
        else if ( funcName == "*" ) {
          ans *= g.GetValueOfFloatStr( argList[ i ] -> lex ) ;
        } // else if()
        else if ( funcName == "/" ) {
          if ( g.GetValueOfFloatStr( argList[ i ] -> lex ) != 0 ) {
            ans /= g.GetValueOfFloatStr( argList[ i ] -> lex ) ;
          } // if()
          else {
            throw new DivideByZeroException() ;
          } // else()
        } // else if()
      } // else if()
    } // for()
    
    if ( ! hasFloatExist ) {
      int intAns = 0 ;
      stringstream sstream ;
      sstream << ans ;
      sstream >> intAns ;
      ansNode -> lex = g.IntToStr( intAns ) ;
    } // if()
    else { // the final answer is a float
      stringstream sstream ;
      sstream << ans ;
      
      // ansNode -> lex = to_string( ans ) ;
      sstream >> ansNode -> lex ;
      if ( g.IsINT( ansNode -> lex ) ) {
        ansNode -> lex = g.FormatIntToFloatStr( ansNode -> lex ) ;
      } // if()
      else {
        ansNode -> lex = g.FormatFloat( ansNode -> lex ) ;
      } // else()
    } // else()
    
    return ansNode ;
  } // ProcessMath()
  
  Node* ProcessCompare( string funcName, vector<Node*> argList, int level ) {
    Node* ansNode = new Node ;
    ansNode -> lex = "#t" ;
    ansNode -> type = SPECIAL ;
    ansNode -> parent = NULL ;
    ansNode -> left = NULL ;
    ansNode -> right = NULL ;
    ansNode -> isAddByMe = false ;
    
    // check whether all the arguments are numbers
    for ( int i = 0 ; i < argList.size() ; i ++ ) {
      Node* currentAug = EvaluateSExp( argList[ i ], ++level ) ;
      if ( currentAug != NULL && currentAug -> type == ATOM
           && ( g.IsINT( currentAug -> lex )
                || g.IsFLOAT( currentAug -> lex ) ) ) {
        argList[ i ] = EvaluateSExp( argList[ i ], ++level ) ;
      } // if()
      else if ( currentAug == NULL ) { // a non number atom exist
        gErrNode = argList[ i ] ;
        throw new ParamNotBoundException() ;
      } // else if()
      else {
        gErrNode = currentAug ;
        throw new IncorrectArgumentTypeException( funcName ) ;
      } // else()
    } // for()
    
    // always compare the adjacent atoms
    double currentValue = 0.0 ;
    if ( g.IsINT( argList[ 0 ] -> lex ) ) {
      currentValue = g.GetValueOfIntStr( argList[ 0 ] -> lex ) ;
    } // if()
    else if ( g.IsFLOAT( argList[ 0 ] -> lex ) ) {
      currentValue = g.GetValueOfFloatStr( argList[ 0 ] -> lex ) ;
    } // else if()
    
    for ( int i = 1 ; i < argList.size() ; i ++ ) {
      double nextValue = 0.0 ;
      if ( g.IsINT( argList[ i ] -> lex ) ) {
        nextValue = g.GetValueOfIntStr( argList[ i ] -> lex ) ;
      } // if()
      else if ( g.IsFLOAT( argList[ i ] -> lex ) ) {
        nextValue = g.GetValueOfFloatStr( argList[ i ] -> lex ) ;
      } // else if()
      
      if ( funcName == ">" ) {
        if ( currentValue > nextValue ) {
          currentValue = nextValue ; // keepp checking the following arguments
        } // if()
        else {
          ansNode -> lex = "nil" ;
          return ansNode ;
        } // else()
      } // if()
      else if ( funcName == ">=" ) {
        if ( currentValue >= nextValue ) {
          currentValue = nextValue ; // keepp checking the following arguments
        } // if()
        else {
          ansNode -> lex = "nil" ;
          return ansNode ;
        } // else()
      } // else if()
      else if ( funcName == "<" ) {
        if ( currentValue < nextValue ) {
          currentValue = nextValue ; // keepp checking the following arguments
        } // if()
        else {
          ansNode -> lex = "nil" ;
          return ansNode ;
        } // else()
      } // else if()
      else if ( funcName == "<=" ) {
        if ( currentValue <= nextValue ) {
          currentValue = nextValue ; // keepp checking the following arguments
        } // if()
        else {
          ansNode -> lex = "nil" ;
          return ansNode ;
        } // else()
      } // else if()
      else if ( funcName == "=" ) {
        if ( currentValue == nextValue ) {
          currentValue = nextValue ; // keepp checking the following arguments
        } // if()
        else {
          ansNode -> lex = "nil" ;
          return ansNode ;
        } // else()
      } // else if()
    } // for()
    
    return ansNode ;
  } // ProcessCompare()
  
  Node* ProcessStringCompare( string funcName, vector<Node*> argList, int level ) {
    Node* ansNode = new Node ;
    ansNode -> lex = "#t" ;
    ansNode -> type = SPECIAL ;
    ansNode -> parent = NULL ;
    ansNode -> left = NULL ;
    ansNode -> right = NULL ;
    ansNode -> isAddByMe = false ;
    
    // check whether all the arguments are numbers
    for ( int i = 0 ; i < argList.size() ; i ++ ) {
      Node* currentAug = EvaluateSExp( argList[ i ], ++level ) ;
      if ( currentAug != NULL
           && currentAug -> type == ATOM && g.IsStr( currentAug -> lex ) ) {
        argList[ i ] = currentAug ;
      } // if()
      else if ( currentAug == NULL ) {
        gErrNode = argList[ i ] ;
        throw new ParamNotBoundException() ;
      } // else if()
      else {
        gErrNode = currentAug ;
        throw new IncorrectArgumentTypeException( funcName ) ;
      } // else()
    } // for()
    
    string currentStr = g.GetStrContent( argList[ 0 ] -> lex ) ;
    string ansStr = currentStr ; // only used in "string-append"
    
    for ( int i = 1 ; i < argList.size() ; i ++ ) {
      if ( funcName == "string-append" ) {
        ansStr += g.GetStrContent( argList[ i ] -> lex ) ;
      } // if()
      else if ( funcName == "string>?" ) {
        if ( currentStr <= g.GetStrContent( argList[ i ] -> lex ) ) {
          ansNode -> lex = "nil" ;
          return ansNode ;
        } // if()
        else {
          currentStr = g.GetStrContent( argList[ i ] -> lex ) ;
        } // else()
      } // else if()
      else if ( funcName == "string<?" ) {
        if ( currentStr >= g.GetStrContent( argList[ i ] -> lex ) ) {
          ansNode -> lex = "nil" ;
          return ansNode ;
        } // if()
        else {
          currentStr = g.GetStrContent( argList[ i ] -> lex ) ;
        } // else()
      } // else if()
      else if ( funcName == "string=?" ) {
        if ( currentStr != g.GetStrContent( argList[ i ] -> lex ) ) {
          ansNode -> lex = "nil" ;
          return ansNode ;
        } // if()
        else {
          currentStr = g.GetStrContent( argList[ i ] -> lex ) ;
        } // else()
      } // else if()
    } // for()
    
    if ( funcName == "string-append" ) {
      ansNode -> type = ATOM ;
      ansNode -> lex = '"' + ansStr + '"' ;
    } // if()
    
    return ansNode ;
  } // ProcessStringCompare()
  
  Node* ProcessCondOperation( string funcName, vector<Node*> argList, int level ) {
    Node* ansNode = new Node ;
    ansNode -> lex = "#t" ;
    ansNode -> type = SPECIAL ;
    ansNode -> parent = NULL ;
    ansNode -> left = NULL ;
    ansNode -> right = NULL ;
    ansNode -> isAddByMe = false ;
    
    bool resultIsTrue = true ;
    for ( int i = 0 ; i < argList.size() ; i ++ ) {
      Node* currentAug = EvaluateSExp( argList[ i ], ++level ) ;
      if ( currentAug != NULL ) {
        argList[ i ] = currentAug ;
      } // if()
      else {
        gErrNode = argList[ i ] ;
        // throw new IncorrectArgumentTypeException( funcName ) ;
        throw new CondNotBoundException() ;
      } // else()
      
      if ( funcName == "not" ) {
        if ( argList[ 0 ] -> lex != "nil" && argList[ 0 ] -> lex != "#f" ) {
          resultIsTrue = false ;
          ansNode -> lex = "nil" ;
          return ansNode ;
        } // if()
      } // if()
      else if ( funcName == "and" ) {
        if ( argList[ i ] -> type == SPECIAL && argList[ i ] -> lex != "#t" ) {
          resultIsTrue = false ;
          ansNode -> lex = "nil" ;
          return ansNode ;
        } // if()
        else if ( i == argList.size() - 1 ) {
          return argList[ i ] ;
        } // else if()
      } // else if()
      else if ( funcName == "or" ) {
        resultIsTrue = false ;
        if ( argList[ i ] -> type != SPECIAL ) {
          return argList[ i ] ;
        } // if()
        else if ( argList[ i ] -> lex == "#t" ) {
          return argList[ i ] ;
        } // else if()
      } // else if()
    } // for()
    
    if ( !resultIsTrue ) {
      ansNode -> lex = "nil" ;
    } // if()
    
    return ansNode ;
  } // ProcessCondOperation()
  
  Node* ProcessOperation( string funcName, Node* inTree, int level ) {
    
    vector<Node*> argList = GetArgumentList( inTree ) ;
    
    if ( IsMathOperator( funcName ) ) { // need to have more than two arguments
      if ( CountArgument( inTree ) >= 2 ) {
        return ProcessMath( funcName, argList, ++level ) ;
      } // if()
      else {
        throw new IncorrectNumberArgumentException( funcName ) ;
      } // else()
    } // if()
    else if ( IsComparison( funcName ) ) { // need to have more than two arguments
      if ( CountArgument( inTree ) >= 2 ) {
        return ProcessCompare( funcName, argList, ++level );
      } // if()
      else {
        throw new IncorrectNumberArgumentException( funcName ) ;
      } // else()
    } // else if()
    else if ( IsCondOperator( funcName ) ) { // not only need 1 argument
      if ( funcName == "not" ) { // only ONE argument
        if ( CountArgument( inTree ) == 1 ) {
          return ProcessCondOperation( funcName, argList, ++level ) ;
        } // if()
        else {
          throw new IncorrectNumberArgumentException( funcName ) ;
        } // else()
      } // if()
      else {
        if ( CountArgument( inTree ) >= 2 ) {
          return ProcessCondOperation( funcName, argList, ++level ) ;
        } // if()
        else {
          throw new IncorrectNumberArgumentException( funcName ) ;
        } // else()
      } // else()
    } // else if()
    else if ( IsStringOperator( funcName ) ) {
      // need to have more than two arguments
      if ( CountArgument( inTree ) >= 2 ) {
        return ProcessStringCompare( funcName, argList, ++level ) ;
      } // if()
      else {
        throw new IncorrectNumberArgumentException( funcName ) ;
      } // else()
    } // else if()
    
    return NULL ;
  } // ProcessOperation()
  
  bool TwoTreesAreTheSame( Node* tree1, Node* tree2 ) {
    if ( tree1 == NULL || tree2 == NULL ) {
      if ( tree1 == tree2 ) {
        return true ;
      } // if()
      
      return false ;
    } // if()
    else {
      if ( tree1 -> lex == tree2 -> lex ) {
        return ( TwoTreesAreTheSame( tree1 -> left, tree2 -> left )
                 && TwoTreesAreTheSame( tree1 -> right, tree2 -> right ) ) ;
      } // if()
      
      return false ;
    } // else()
  } // TwoTreesAreTheSame()
  
  Node* ProcessEqvAndEqual( string funcName, Node* inTree, int level ) {
    Node* ansNode = new Node ;
    ansNode -> lex = "#t" ;
    ansNode -> type = SPECIAL ;
    ansNode -> parent = NULL ;
    ansNode -> left = NULL ;
    ansNode -> right = NULL ;
    ansNode -> isAddByMe = false ;
    
    if ( CountArgument( inTree ) == 2 ) {
      vector<Node*> argList = GetArgumentList( inTree ) ;
      
      for ( int i = 0 ; i < argList.size() ; i ++ ) {
        EvaluateSExp( argList[ i ], ++level ) ; // only used to check whether is any wrong
      } // for()
      
      if ( funcName == "eqv?" ) { // compare the pointer
        bool isInSameMemory = false ;
        
        if ( argList[ 0 ] -> type != CONS && argList[ 1 ] -> type != CONS
             && g.GetTokenType( argList[ 0 ] -> lex ) == SYMBOL
             && g.GetTokenType( argList[ 1 ] -> lex ) == SYMBOL ) {
          int symIndex1 = FindSymbolFromLocalAndGlobal( argList[ 0 ] -> lex ) ;
          int symIndex2 = FindSymbolFromLocalAndGlobal( argList[ 1 ] -> lex ) ;
          if ( symIndex1 != -1 && symIndex2 != -1 ) {
            // case1. both of them are local
            if ( mCallStack.IsLocalVar( argList[ 0 ] -> lex )
                 && mCallStack.IsLocalVar( argList[ 1 ] -> lex ) ) {
              if ( mCallStack.GetLocalVarBinding( argList[ 0 ] -> lex )
                   == mCallStack.GetLocalVarBinding( argList[ 1 ] -> lex ) ) {
                isInSameMemory = true ;
              } // if()
            } // if()
            // case2. both of them are global
            else if ( !mCallStack.IsLocalVar( argList[ 0 ] -> lex )
                      && !mCallStack.IsLocalVar( argList[ 1 ] -> lex ) ) {
              if ( mSymbolTable[ symIndex1 ].tree == mSymbolTable[ symIndex2 ].tree ) {
                isInSameMemory = true ;
              } // if()
            } // else if()
          } // if()
        } // if()
        else if ( g.IsINT( argList[ 0 ] -> lex )
                  && g.IsINT( argList[ 1 ] -> lex ) ) {
          if ( g.GetValueOfIntStr( argList[ 0 ] -> lex )
               == g.GetValueOfIntStr( argList[ 1 ] -> lex ) ) {
            isInSameMemory = true ;
          } // if()
        } // else if()
        else if ( g.IsFLOAT( argList[ 0 ] -> lex )
                  && g.IsFLOAT( argList[ 1 ] -> lex ) ) {
          if ( g.GetValueOfFloatStr( argList[ 0 ] -> lex )
               == g.GetValueOfFloatStr( argList[ 1 ] -> lex ) ) {
            isInSameMemory = true ;
          } // if()
        } // else if()
        else { // check the #t and #f and nil and '()
          Node* tmp1 = EvaluateSExp( argList[ 0 ], ++level ) ;
          Node* tmp2 = EvaluateSExp( argList[ 1 ], ++level ) ;
          if ( tmp1 -> type == SPECIAL && tmp2 -> type == SPECIAL ) {
            if ( tmp1 -> lex == tmp2 -> lex ) {
              isInSameMemory = true ;
            } // if()
          } // if()
        } // else()
        
        if ( isInSameMemory ) {
          ansNode -> lex = "#t" ;
          return ansNode ;
        } // if()
        else {
          ansNode -> lex = "nil" ;
          return ansNode ;
        } // else()
      } // if()
      else if ( funcName == "equal?" ) { // compare the context
        // string originArgStr1 = argList[ 0 ] -> lex ;
        // string originArgStr2 = argList[ 1 ] -> lex ;
        argList[ 0 ] = EvaluateSExp( argList[ 0 ], ++level ) ;
        argList[ 1 ] = EvaluateSExp( argList[ 1 ], ++level ) ;
        
        if ( argList[ 0 ] == NULL ) {
          gErrNode = argList[ 0 ] ;
          // throw new IncorrectArgumentTypeException( funcName ) ;
          throw new ParamNotBoundException() ;
        } // if()
        else if ( argList[ 1 ] == NULL ) {
          gErrNode = argList[ 1 ] ;
          // throw new IncorrectArgumentTypeException( funcName ) ;
          throw new ParamNotBoundException() ;
        } // else if()
        else {
          if ( ! TwoTreesAreTheSame( argList[ 0 ], argList[ 1 ] ) ) {
            ansNode -> lex = "nil" ;
          } // if()
        } // else()
      } // else if()
    } // if()
    else {
      throw new IncorrectNumberArgumentException( funcName ) ;
    } // else()
    
    return ansNode ;
  } // ProcessEqvAndEqual()
  
  Node* ProcessIf( Node* inTree, int level ) {
    // has two or three arguments
    if ( CountArgument( inTree ) == 2 || CountArgument( inTree ) == 3 ) {
      vector<Node*> argList = GetArgumentList( inTree ) ;
      
      // the first arguments should be the condition
      // if the evaluate of argment 1 is NULL then the format is wrong
      Node* condition = EvaluateSExp( argList[ 0 ], ++level ) ;
      if ( condition != NULL ) {
        if ( CountArgument( inTree ) == 2 ) {
          if ( condition -> lex != "#f" && condition -> lex != "nil" ) {
            return EvaluateSExp( argList[ 1 ], ++level ) ;
          } // if()
          else ;
          /*
          else {
            gErrNode = inTree ;
            throw new NoReturnValueException() ;
          } // else()
          */
        } // if()
        else if ( CountArgument( inTree ) == 3 ) {
          if ( condition -> lex != "#f" && condition -> lex != "nil" ) {
            return EvaluateSExp( argList[ 1 ], ++level ) ;
          } // if()
          else {
            return EvaluateSExp( argList[ 2 ], ++level ) ;
          } // else()
        } // else if()
      } // if()
      else {
        gErrNode = argList[ 0 ] ;
        // throw new IncorrectArgumentTypeException( "if" ) ;
        throw new TestCondNotBoundException() ;
      } // else()
    } // if()
    else {
      throw new IncorrectNumberArgumentException( "if" ) ; // curious here
    } // else()
    
    return NULL ;
  } // ProcessIf()
  
  Node* ProcessCond( Node* inTree, int level ) {
    // cond expression cam take more than 1 arguments, at least one
    
    if ( CountArgument( inTree ) >= 1 ) {
      vector<Node*> argList = GetArgumentList( inTree ) ;
      
      // check each arguments should all be cons
      for ( int i = 0 ; i < argList.size() ; i ++ ) {
        if ( argList[ i ] -> type != CONS
             || ! IsList( argList[ i ] )
             || CountArgument( argList[ i ] ) == 0 ) {
          gErrNode = inTree ;
          throw new CondFormatException() ;
        } // if()
      } // for()
      
      // start finding the first satisfying statement
      for ( int i = 0 ; i < argList.size() ; i ++ ) {
        Node* condResult = NULL ;
        Node* condPart = argList[ i ] -> left ;
        Node* statePart = NULL ;
        vector<Node*> subAugList = GetArgumentList( argList[ i ] ) ;
        
        if ( i < argList.size() - 1 ) {
          condResult = EvaluateSExp( condPart, ++level ) ;
          if ( condResult != NULL ) {
            if ( condResult -> lex != "#f"
                 && condResult -> lex != "nil"  ) {
              // assert: has deal with all the additional statement
              for ( int subI = 0 ; subI < subAugList.size() ; subI ++ ) {
                if ( subI == subAugList.size() - 1 ) {
                  statePart = subAugList[ subI ] ;
                } // if()
                else {
                  EvaluateSExp( subAugList[ subI ], ++level ) ;
                } // else()
              } // for()
              
              return EvaluateSExp( statePart, ++level ) ;
            } // if()
          } // if()
          else {
            gErrNode = inTree ;
            throw new CondFormatException() ;
          } // else()
        } // if()
        else {
          // the last condition can start with the key word "else"
          if ( condPart -> lex == "else" ) { // don't need to evaluate
            // assert: has deal with all the additional statement
            for ( int subI = 0 ; subI < subAugList.size() ; subI ++ ) {
              if ( subI == subAugList.size() - 1 ) {
                statePart = subAugList[ subI ] ;
              } // if()
              else {
                EvaluateSExp( subAugList[ subI ], ++level ) ;
              } // else()
            } // for()
            
            return EvaluateSExp( statePart, ++level );
          } // if()
          else {
            condResult = EvaluateSExp( condPart, ++level ) ;
            if ( condResult != NULL ) {
              if ( condResult -> lex != "#f"
                   && condResult -> lex != "nil" ) {
                // assert: has deal with all the additional statement
                for ( int subI = 0 ; subI < subAugList.size() ; subI ++ ) {
                  if ( subI == subAugList.size() - 1 ) {
                    statePart = subAugList[ subI ] ;
                  } // if()
                  else {
                    EvaluateSExp( subAugList[ subI ], ++level ) ;
                  } // else()
                } // for()
                
                if ( statePart -> type != EMPTY ) {
                  return EvaluateSExp( statePart, ++level ) ;
                } // if()
                else {
                  gErrNode = inTree ;
                  throw new CondFormatException() ;
                } // else()
              } // if()
              else if ( condResult == NULL || condResult -> type == EMPTY ) {
                // gErrNode = inTree ;
                // throw new NoReturnValueException() ; // curious
                gErrNode = condPart ;
                throw new TestCondNotBoundException() ;
              } // else if()
            } // if()
            else {
              gErrNode = inTree ;
              throw new CondFormatException() ; // curious
            } // else()
          } // else()
        } // else()
      } // for()
    } // if()
    else {
      gErrNode = inTree ;
      throw new CondFormatException() ; // curious
    } // else()
    
    // gErrNode = inTree ;
    // throw new NoReturnValueException() ; // curious
    
    return NULL ;
  } // ProcessCond()
  
  Node* ProcessBegin( Node* inTree, int level ) {
    if ( CountArgument( inTree ) >= 1 ) {
      // sequencing evaluate all argements, but return the final one
      vector<Node*> argList = GetArgumentList( inTree ) ;
      
      for ( int i = 0 ; i < argList.size() ; i ++ ) {
        if ( !IsList( argList[ i ] ) && argList[ i ] -> type == CONS ) {
          gErrNode = argList[ i ] ;
          throw new IncorrectArgumentTypeException( "begin" ) ;
        } // if()
      } // for()
      
      for ( int i = 0 ; i < argList.size() ; i ++ ) {
        if ( i == argList.size() - 1 ) {
          return EvaluateSExp( argList[ i ], ++level ) ;
        } // if()
        else {
          EvaluateSExp( argList[ i ], ++level ) ;
        } // else()
      } // for()
    } // if()
    else {
      throw new IncorrectNumberArgumentException( "begin" ) ;
    } // else()
    
    return NULL ;
  } // ProcessBegin()
  
  Node* ProcessVerbose( string funcName, Node* inTree, int level ) {
    if ( funcName == "verbose" ) {
      if ( CountArgument( inTree ) == 1 ) {
        Node* arg = EvaluateSExp( inTree -> right -> left, ++level ) ;
        if ( arg -> type == SPECIAL && arg -> lex == "nil" ) {
          gVerbose = false ;
          return g.GetNullNode() ;
        } // if()
        else {
          gVerbose = true ;
          Node* node = g.GetEmptyNode() ;
          node -> lex = "#t" ;
          node -> type = SPECIAL ;
          return node ;
        } // else()
      } // if()
      else {
        throw new IncorrectNumberArgumentException( "verbose" ) ;
      } // else()
    } // if()
    else if ( funcName == "verbose?" ) {
      if ( CountArgument( inTree ) == 0 ) {
        if ( gVerbose ) {
          Node* node = g.GetEmptyNode() ;
          node -> lex = "#t" ;
          node -> type = SPECIAL ;
          return node ;
        } // if()
        else {
          return g.GetNullNode() ;
        } // else()
      } // if()
      else {
        throw new IncorrectNumberArgumentException( "verbose?" ) ;
      } // else()
    } // else if()
    
    return NULL ;
  } // ProcessVerbose()
  
  bool CheckAndStoreLocalVarSuccess( Node* localVars, vector<string> &varNameList, int level ) {
    vector<Node*> varList ;
    vector<Node*> bindingList ;
    // Seperate all the local variables from the tree structure into a vactor
    if ( IsList( localVars ) ) {
      for ( Node* walk = localVars ; walk -> lex != "nil" ; walk = walk -> right ) {
        if ( IsList( walk -> left ) && CountListElement( walk -> left ) == 2 ) {
          varNameList.push_back( walk -> left -> left -> lex ) ;
          varList.push_back( walk -> left ) ;
        } // if()
        else {
          return false ;
        } // else()
      } // for()
    } // if()
    else {
      return false ;
    } // else()
    
    if ( varList.size() == 1 && varList[ 0 ] == NULL ) {
      return true ;
    } // if()
    
    for ( int i = 0 ; i < varList.size() ; i ++ ) {
      if ( varList[ i ] -> type == CONS ) { // Should be a cons structure
        
        string varName = varList[ i ] -> left -> lex ;
        Node* bind = varList[ i ] -> right -> left ;
        
        if ( g.IsSymbol( varName ) ) { // local variable should be a symbol
          
          Node* binding = NULL ;
          binding = EvaluateSExp( bind, ++level ) ;
          CheckHasReturnBindingOrThrow( binding, bind ) ;
          
          if ( binding == NULL || binding -> type == EMPTY ) {
            varList.clear() ;
            
            gErrNode = varList[ i ] -> right ;
            throw new NonReturnAssignedException() ;
          } // if()
          else {
            bindingList.push_back( binding ) ;
          } // else()
        } // if()
        else {
          varList.clear() ;
          return false ;
        } // else()
      } // if()
      else {
        varList.clear() ;
        return false ;
      } // else()
    } // for()
    
    // assert: all parameter binding should benn evaluated
    mCallStack.AddLocalVars( varNameList, bindingList, level ) ;
    
    varList.clear() ;
    bindingList.clear() ;
    
    return true ;
  } // CheckAndStoreLocalVarSuccess()
  
  Node* ProcessLet( Node* inTree, int level ) {
    Node* allArg = inTree -> right ;
    Node* localVarList = allArg -> left ;
    Node* allSExp = allArg -> right ;
    vector<string> localVarNameList ;
    
    if ( CountArgument( inTree ) >= 2 ) {
      mCallStack.GetCleanLocalZone() ;
      
      if ( CheckAndStoreLocalVarSuccess( localVarList, localVarNameList, ++level ) ) {
        
        Node* walk ;
        for ( walk = allSExp ; walk -> right -> lex != "nil" ; walk = walk -> right ) {
          EvaluateSExp( walk -> left, ++level ) ;
        } // for()
        
        Node* finalResult = EvaluateSExp( walk -> left, ++level ) ; // the last expression result

        mCallStack.RestoreLocalVar( localVarNameList ) ;
        mCallStack.ClearCurrentLocalVar() ;
        
        return finalResult ;
      } // if()
      else ;
    } // if()
    
    gErrNode = inTree ;
    throw new FormatException( "LET" ) ;
    return NULL ;
    
  } // ProcessLet()
  
  // use when the defining lambda
  int CountAndCkeckParameters( Node* arg, vector<string> &paraList ) {
    int countNum = CountListElement( arg ) ;
    paraList.clear() ;
    
    if ( IsList( arg ) ) { // the parameter part is a list
      for ( Node* walk = arg ; walk -> lex != "nil" ; walk = walk -> right ) {
        if ( walk -> left -> type == ATOM ) {
          if ( g.IsSymbol(  walk -> left -> lex ) ) {
            paraList.push_back( walk -> left -> lex ) ;
          } // if()
          else {
            gErrNode = arg ;
            throw new FormatException( "LAMBDA" ) ;
          } // else()
        } // if()
        else {
          gErrNode = arg ;
          throw new FormatException( "LAMBDA" ) ;
        } // else()
      } // for()
    } // if()
    else {
      gErrNode = arg ;
      throw new FormatException( "LAMBDA" ) ;
    } // else()
    
    return countNum ;
  } // CountAndCkeckParameters()
  
  int CountAndCkeckParameters( Node* arg, vector<Node*> &paraList, int level ) {
    int countNum = CountListElement( arg ) ;
    paraList.clear() ;
    
    for ( Node* walk = arg ; walk -> lex != "nil" ; walk = walk -> right ) {
      paraList.push_back( walk -> left ) ;
    } // for()
    
    return countNum ;
  } // CountAndCkeckParameters()
  
  void ParameterBinding( vector<string> paramList, Node* bindings, string processName, int level ) {
    vector<Node*> bindingList ;
    vector<Node*> tmpBindingList ;
    int parNum = 0 ;
    parNum = CountAndCkeckParameters( bindings, bindingList, ++level ) ;
    
    if ( paramList.size() == bindingList.size() ) {
      for ( int i = 0 ; i < paramList.size() ; i ++ ) {
        // the argument should be evaluate first, before binding to the symbol
        Node* bindValue = EvaluateSExp( bindingList[ i ], ++level ) ;
        if ( bindValue != NULL ) {
          tmpBindingList.push_back( bindValue ) ;
        } // if()
        else {
          gErrNode = bindingList[ i ] ;
          throw new ParamNotBoundException() ;
        } // else()
        
      } // for()
      
      mCallStack.AddLocalVars( paramList, tmpBindingList, level ) ;
      
      bindingList.clear() ;
      tmpBindingList.clear() ;
    } // if()
    else {
      bindingList.clear() ;
      tmpBindingList.clear() ;
      throw new IncorrectNumberArgumentException( processName ) ;
    } // else()
  } // ParameterBinding()
  
  Node* ProcessLambda( Node* inTree, int level ) {
    mCallStack.GetCleanLocalZone() ;
    // When in this function, there might be two circumstaces
    // 1. Not yet be evaluated
    // 2. Is the returned #<procedure lambda
    Node* lambdaProc ;
    int reserveIndex = FindGlobalSymbol( "lambda" ) ;
    lambdaProc = mSymbolTable[ reserveIndex ].tree ;
    
    if ( IsUserDefinedFunc( inTree -> left -> lex ) ) {
      return ProcessUserDefinedFunc( inTree, FindUserDefinedFunc( inTree -> left -> lex ), ++level );
    } // if()
    else if ( inTree -> left -> type == CONS ) { // the second circumstances
      mCallStack.GetCleanLocalZone() ;
      Node* finalResult = NULL ;
      
      if ( inTree -> right -> lex != "nil" ) { // immediately call the lambda function
        mLambdaStack.push_back( mLambdaFunc ) ;
        
        try {
          ParameterBinding( mLambdaFunc.argList, inTree -> right, "lambda", ++level ) ;
        } catch ( NoReturnValueException* e ) {
          gErrNode = inTree ;
          throw new NoReturnValueException() ;
        } // catch()
        
        mLambdaFunc = mLambdaStack[ mLambdaStack.size() - 1 ] ;
        mLambdaStack.pop_back() ;
      } // if()
      
      if ( mLambdaFunc.tree != NULL ) {
        for ( Node* walk = mLambdaFunc.tree ; walk -> lex != "nil" ; walk = walk -> right ) {
          if ( walk -> right -> lex == "nil" ) {
            finalResult = EvaluateSExp( walk -> left, ++level ) ;
          } // if()
          else {
            EvaluateSExp( walk -> left, ++level ) ;
          } // else()
        } // for()
      } // if()
      
      mCallStack.RestoreLocalVar( mLambdaFunc.argList ) ;
      mCallStack.ClearCurrentLocalVar() ;
      
      return finalResult ;
    } // else if()
    else {
      // the left node of the input tree is "lambda", means this is the procedure of defineing
      // a lambda function
      
      if ( CountArgument( inTree ) >= 2 ) {
        Node* allArg = inTree -> right ;
        Node* localVarList = allArg -> left ;
        Node* allSExp = allArg -> right ;
        
        if ( localVarList -> type == CONS
             || ( localVarList -> type == SPECIAL && localVarList -> lex == "nil" ) ) {
          
          try {
            
            mLambdaFunc.argNum = CountAndCkeckParameters( localVarList, mLambdaFunc.argList ) ;
            // mLambdaFunc.tree = allSExp ;
            mLambdaFunc.tree = CopyTree( allSExp ) ;
            
          } catch ( FormatException* e ) {
            gErrNode = inTree ;
            throw new FormatException( "LAMBDA" ) ;
          } // catch()
          
          return lambdaProc ;
        } // if()
        else {
          gErrNode = inTree ;
          throw new FormatException( "LAMBDA" ) ;
        } // else()
      } // if()
      
    } // else()
    
    gErrNode = inTree ;
    throw new FormatException( "LAMBDA" ) ;
    return NULL ;
  } // ProcessLambda()
  
  Node* ProcessUserDefinedFunc( Node* inTree, int funcIndex, int level ) {
    mCallStack.GetCleanLocalZone() ;
    
    Function func = mUserDefinedFunctionTable[ funcIndex ] ;
    Node* treeInSymbolTable = mSymbolTable[ FindSymbolFromLocalAndGlobal( func.name ) ].tree ;
    Node* finalResult = NULL ;
    vector<string> paramList ;
    
    if ( treeInSymbolTable -> type == ATOM && treeInSymbolTable -> lex == "#<procedure lambda>" ) {
      ParameterBinding( func.argList, inTree -> right, "lambda", ++level ) ;
    } // if()
    else {
      ParameterBinding( func.argList, inTree -> right, func.name, ++level ) ;
    } // else()
    
    // copy the local variables
    // after processing this S-exp, these local cariables need to be erased
    paramList.assign( func.argList.begin(), func.argList.end() ) ;
    
    for ( Node* walk = func.tree ; walk -> lex != "nil" ; walk = walk -> right ) {
      if ( walk -> right -> lex == "nil" ) {
        finalResult = EvaluateSExp( walk -> left, ++level ) ;
      } // if()
      else {
        EvaluateSExp( walk -> left, ++level ) ;
      } // else()
    } // for()
    
    mCallStack.RestoreLocalVar( func.argList ) ;
    mCallStack.ClearCurrentLocalVar() ;
    return finalResult ;
  } // ProcessUserDefinedFunc()
  
  void AddOriginReserveWords() {
    
    for ( int i = 0 ; i < gReserveWordNum ; i ++ ) {
      Node* tmpNode = new Node ;
      tmpNode -> lex = "#<procedure " + g.GetStrContent( gOriginReserveWordList[ i ] ) + ">" ;
      tmpNode -> type = ATOM ;
      tmpNode -> parent = NULL ;
      tmpNode -> left = NULL ;
      tmpNode -> right = NULL ;
      tmpNode -> isAddByMe = false ;
      
      Symbol tmpSym ;
      tmpSym.name = gOriginReserveWordList[ i ] ;
      tmpSym.tree = tmpNode ;
      
      mSymbolTable.push_back( tmpSym ) ;
    } // for()
  } // AddOriginReserveWords()
  
  void ResetSymbolTable() {
    mSymbolTable.erase( mSymbolTable.begin() + gReserveWordNum, mSymbolTable.end() ) ;
  } // ResetSymbolTable()
  
  string GetFuncNameFromFuncValue( string str ) {
    string name = "" ;
    
    if ( str[ 0 ] == '#' ) {
      int whiteIndex = ( int ) str.find( ' ', 0 ) ;
      for ( int i = whiteIndex + 1 ; i < str.length() ; i ++ ) {
        if ( i != str.length() - 1 ) {
          name += str[ i ] ;
        } // if()
      } // for()
    } // if()
    else {
      name = str ;
    } // else()
    
    return name ;
  } // GetFuncNameFromFuncValue()
  
  void InitialLambdaFunc() {
    mLambdaFunc.name = "" ;
    mLambdaFunc.argNum = 0 ;
    mLambdaFunc.tree = NULL ;
  } // InitialLambdaFunc()
  
public:
  Evaluator() {
    AddOriginReserveWords() ;
    InitialReserveWord() ;
    InitialLambdaFunc() ;
  } // Evaluator()
  
  bool CheckHasReturnBinding( Node* result, Node* binding ) {
    if ( result == NULL ) {
      gErrNode = binding ;
      return false ;
    } // if()
    else ;
    
    return true ;
  } // CheckHasReturnBinding()
  
  bool CheckHasReturnBindingOrThrow( Node* result, Node* binding ) {
    if ( result == NULL ) {
      gErrNode = binding ;
      throw new NoReturnValueException() ;
    } // if()
    else ;
    
    return true ;
  } // CheckHasReturnBindingOrThrow()
  
  bool CheckTopLevelHasReturnBinding( Node* result, Node* binding ) {
    if ( result == NULL ) {
      gErrNode = binding ;
      return false ;
    } // if()
    else ;
    
    return true ;
  } // CheckTopLevelHasReturnBinding()
  
  Node* EvaluateSExp( Node* treeRoot, int level ) {
    
    // the first left atom should be the func name
    Node* result = NULL ; // used to store the evaluation result tree
    string originFuncName = "" ; // copy the original operator from the fiven tree
    
    // Local vairables in each zone (S-exp) cannot be push until all evaluation is done
    
    if ( treeRoot == NULL ) { // to make sure the recent evaluated tree is not null
      return NULL ;
    } // if()
    else ; // the input tree is not empty, keep evaluating
    
    // Two conditions in evaluating S-exp
    // 1. Atom
    // 2. CONS
    if ( treeRoot -> type == ATOM ) { // This S-exp is an Atom
      // may be p pure number or a symbol
      string atomStr = treeRoot -> lex ;
      // this S-exp is a number
      if ( g.IsINT( atomStr ) || g.IsFLOAT( atomStr ) || g.IsStr( atomStr ) ) {
        result = treeRoot ;
      } // if()
      else if ( SymbolExist( atomStr ) ) { // this S-exp is a exist symbol
        result = FindSymbolBinding( atomStr ) ;
      } // else if()
      else {
        throw new UnboundValueException( atomStr ) ;
      } // else()
      
      return result ;
    } // if()
    else if ( treeRoot -> type == SPECIAL ) {
      return treeRoot ;
    } // else if()
    else if ( treeRoot -> type == CONS ) { // This S-exp is a CONS
      Node* funcPart = treeRoot -> left ; // the left node of the root must be function part
      
      if ( funcPart -> type == ATOM ) { // if this is an atom, then check what it represents
        if ( SymbolExist( funcPart -> lex ) ) {
          originFuncName = FindSymbolBinding( funcPart -> lex ) -> lex ;
        } // if()
        else {
          originFuncName = funcPart -> lex ;
        } // else()
      } // if()
      else if ( funcPart -> type == CONS ) {
        Node* funcResult = EvaluateSExp( funcPart, ++level ) ;
        CheckHasReturnBindingOrThrow( funcResult, funcPart ) ;
        originFuncName = funcResult -> lex ;
      } // else if()
      else { // the function part is neither an atom, not a CONS
        gErrNode = treeRoot ;
        throw new ApplyNonFunctionException() ;
      } // else()
    } // else if()
    else { // This is S-exp must be an Error
      gErrNode = treeRoot ;
      throw new ApplyNonFunctionException() ;
    } // else()
    
    // originFuncName = GetFuncNameFromFuncValue( originFuncName ) ;
    // After evaluate the function name part, now we can decide which function to process
    string funcName = "" ;
    int definedFuncIndex = -1 ;
    if ( originFuncName[ 0 ] == '#' || originFuncName == "lambda" ) {
      // the function name after evaluation ( if the original one is a symbol or some how)
      funcName = GetReserveWordType( originFuncName ) ;
      // the functions are stored in mFuncTable, consist of the function name and definition
      definedFuncIndex = FindUserDefinedFunc( originFuncName ) ;
    } // if()
    else ; // this function name is not start with the symbol #
    
    if ( IsList( treeRoot ) ) { // keep doing the evaluation
      
      if ( funcName != "" ) {
        if ( funcName == "quote" ) {
          result = treeRoot -> right -> left ;
        } // if()
        else if ( funcName == "cons" ) {
          result = EvaluateCONS( treeRoot, ++level ) ;
        } // else if()
        else if ( funcName == "list" ) {
          result = EvaluateLIST( treeRoot, ++level ) ;
        } // else if()
        else if ( funcName == "define" ) {
          result = Define( treeRoot, ++level ) ;
        } // else if()
        else if ( funcName == "car" || funcName == "cdr" ) {
          result = AccessList( funcName, treeRoot, ++level ) ;
        } // else if()
        else if ( IsPredicator( funcName ) ) {
          result = PrimitivePredecates( funcName, treeRoot, ++level ) ;
        } // else if()
        else if ( IsBasicOperation( funcName ) ) {
          result = ProcessOperation( funcName, treeRoot, ++level );
        } // else if()
        else if ( funcName == "eqv?" || funcName == "equal?" ) {
          result = ProcessEqvAndEqual( funcName, treeRoot, ++level );
        } // else if()
        else if ( funcName == "if" ) {
          result = ProcessIf( treeRoot, ++level ) ;
        } // else if()
        else if ( funcName == "cond" ) {
          result = ProcessCond( treeRoot, ++level ) ;
        } // else if()
        else if ( funcName == "begin" ) {
          result = ProcessBegin( treeRoot, ++level ) ;
        } // else if()
        else if ( funcName == "let" ) {
          result = ProcessLet( treeRoot, ++level ) ;
        } // else if()
        else if ( funcName == "lambda" ) {
          result = ProcessLambda( treeRoot, ++level ) ;
        } // else if()
        else if ( funcName == "verbose" || funcName == "verbose?" ) {
          result = ProcessVerbose( funcName, treeRoot, ++level ) ;
        } // else if()
        else if ( funcName == "clean-environment" ) {
          if ( level == 0 ) {
            CleanEnvironment() ;
          } // if()
          else {
            throw new LevelException( "CLEAN-ENVIRONMENT" ) ;
          } // else()
          
          result = NULL ; // no tree to return
        } // else if()
        else if ( funcName == "exit" ) {
          if ( level != 0 ) {
            throw new LevelException( "EXIT" ) ;
          } // if()
          else if ( CountArgument( treeRoot ) != 0 ) {
            throw new IncorrectNumberArgumentException( funcName ) ;
          } // else if()
          
          gIsEOF = true ;
          result = NULL ;
        } // else if()
      } // if()
      else if ( definedFuncIndex != -1 ) { // this user-defined function exist
        // process the user defined function
        result = ProcessUserDefinedFunc( treeRoot, definedFuncIndex, ++level ) ;
      } // else if()
      else { // either a function name or a symbol
        gErrNode = EvaluateSExp( treeRoot -> left, ++level ) ;
        throw new ApplyNonFunctionException() ; // curious
      } // else()
    } // if()
    else {
      gErrNode = treeRoot ;
      throw new NonListException() ; // curious
    } // else()
    
    if ( level == 1 && result != NULL ) {
      if ( result -> lex == "lambda" ) {
        result -> lex = "#<procedure lambda>" ;
      } // if()
    } // if()
    
    return result ;
  } // EvaluateSExp()
  
  void CleanLocalVariables() {
    mCallStack.ClearCurrentLocalVar() ;
    InitialLambdaFunc() ;
  } // CleanLocalVariables()
  
  void CleanWholeStack() {
    mCallStack.CleanStack() ;
  } // CleanWholeStack()
  
} ; // Evaluator

bool IsDefineOrCleanSExp( Node* node ) {
  if ( node -> type == CONS ) {
    if ( node -> left -> lex == "define" || node -> left -> lex == "clean-environment" ) {
      return true ;
    } // if()
    else ;
  } // if()
  else ;
  
  return false ;
} // IsDefineOrCleanSExp()

int main() {
  
  LexicalAnalyzer la ;
  SyntaxAnalyzer sa ;
  Evaluator eval ;
  bool grammerCorrect = false ;
  
  cin >> uTestNum ;
  char retuenLine = cin.get() ;
  
  cout << "Welcome to OurScheme!" << endl ;
  string inputStr = "" ;
  
  while ( !gIsEOF ) {
    
    try {
      
      cout << endl << "> " ;
      la.PeekToken() ;
      Token token = la.GetToken() ;
      
      grammerCorrect = sa.CheckSExp( token ) ;
      if ( grammerCorrect ) {
        Tree tree( gOriginalList ) ;
        tree.BuildTree() ;
        g.Reset() ;
        
        if ( !gIsEOF ) {
          // g.PrettyPrint( tree.GetRoot() ) ; // proj.1
          try {
            // Evaluate the tree and start with level 0
            Node* result = eval.EvaluateSExp( tree.GetRoot(), 0 ) ;
            // check whether the result has a binding
            // Two exceptions: 1.define 2. clean-environment
            if ( !eval.CheckTopLevelHasReturnBinding( result, tree.GetRoot() )
                 && !IsDefineOrCleanSExp( tree.GetRoot() ) ) {
              gErrNode = tree.GetRoot() ;
              throw new NoReturnValueException() ;
            } // if()
            else ;
            
            if ( result != NULL ) {
              g.PrettyPrint( result ) ;
            } // if()
          } // try
          catch ( LevelException* e ) {
            cout << e -> Err_mesg() << endl ;
          } // catch()
          catch ( NonListException* e ) {
            cout << e -> Err_mesg() ;
            g.PrettyPrint( gErrNode ) ;
            // cout << endl ;
          } // catch()
          catch ( UnboundValueException* e ) {
            cout << e -> Err_mesg() << endl ;
          } // catch()
          catch ( ApplyNonFunctionException* e ) {
            cout << e -> Err_mesg() ;
            g.PrettyPrint( gErrNode ) ;
          } // catch()
          catch ( IncorrectNumberArgumentException* e ) {
            cout << e -> Err_mesg() << endl ;
          } // catch()
          catch ( IncorrectArgumentTypeException* e ) {
            cout << e -> Err_mesg() ;
            g.PrettyPrint( gErrNode ) ;
          } // catch()
          catch ( NoReturnValueException* e ) {
            cout << e -> Err_mesg() ;
            g.PrettyPrint( gErrNode ) ;
            // cout << endl ;
          } // catch()
          catch ( ParamNotBoundException* e ) {
            cout << e -> Err_mesg() ;
            g.PrettyPrint( gErrNode ) ;
          } // catch()
          catch ( DivideByZeroException* e ) {
            cout << e -> Err_mesg() << endl ;
          } // catch()
          catch ( DefineFormatException* e ) {
            // cout << e -> Err_mesg() << endl ;
            cout << e -> Err_mesg() ;
            g.PrettyPrint( gErrNode ) ;
          } // catch()
          catch ( CondFormatException* e ) {
            cout << e -> Err_mesg() ;
            g.PrettyPrint( gErrNode ) ;
            // cout << endl ;
          } // catch()
          catch ( CondNotBoundException* e ) {
            cout << e ->Err_mesg() ;
            g.PrettyPrint( gErrNode ) ;
          } // catch()
          catch ( TestCondNotBoundException* e ) {
            cout << e ->Err_mesg() ;
            g.PrettyPrint( gErrNode ) ;
          } // catch()
          catch ( FormatException* e ) {
            cout << e ->Err_mesg() ;
            g.PrettyPrint( gErrNode ) ;
          } // catch()
          catch ( NonReturnAssignedException* e ) {
            cout << e -> Err_mesg() ;
            g.PrettyPrint( gErrNode ) ;
          } // catch()
        } // if()
      } // if()
      
      // gOriginalList.Clear() ;
      // g.Reset() ;
      gJustFinishAExp = true ;
      eval.CleanWholeStack() ;
    } // catch()
    catch ( EOFException* e ) {
      cout << e -> Err_mesg() << endl ;
      gJustFinishAExp = true ;
      cout << "Thanks for using OurScheme!" << endl ;
      return 0 ;
    } // catch()
    catch ( MissingAtomOrLeftParException* e ) {
      cout << e -> Err_mesg() << endl ;
      gJustFinishAExp = true ;
    } // catch()
    catch ( MissingRightParException* e ) {
      cout << e -> Err_mesg() << endl ;
      gJustFinishAExp = true ;
    } // catch()
    catch ( NoClosingQuoteException* e ) {
      cout << e -> Err_mesg() << endl ;
      gJustFinishAExp = true ;
    } // catch()
    
  } // while()
  
  gOriginalList.Clear() ;
  g.Reset() ;
  
  cout << endl << "Thanks for using OurScheme!" ;
  
} // main()
