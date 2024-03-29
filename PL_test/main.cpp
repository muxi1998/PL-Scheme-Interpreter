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
// INT 12*97  e.gG., '123', '+123', '-123'
// STRING 14*97  doesn't cross line, inclue following char >\n<, >\"<, >\t<, >\\<
// DOT 33*97 '.'
// FLOAT 16*97 '123.567', '123.', '.567', '+123.4' , '-.123'
// NIL 13*97 'nil', '#f' only these two possible look
// T 26*97 't', '#t' only these two possible look, too
// QUOTE 44*97 '
// SYMBOL 18*97  // DO NOT contain '(', ')', '\'', '\"', white-space
# define RNUM 61

string gOriginReserveWordList[ RNUM ] = { "cons", "list", "quote", "define"
  , "car", "cdr", "not", "and", "or", "begin", "if", "cond"
  ,  "clean-environment", "quote", "'", "atom?", "pair?", "list?"
  , "null?", "integer?", "real?", "number?", "string?",  "boolean?"
  , "symbol?", "+", "-", "*", "/", ">", ">=", "<", "<=", "=", "and"
  , "not", "or", "string-append", "string>?", "string<?", "string=?"
  , "eqv?", "equal?", "begin", "if", "cond", "exit", "lambda", "let", "verbose", "verbose?"
  , "create-error-object", "error-object?", "read", "write", "display-string", "newline", "eval"
  , "set!", "symbol->string", "number->string" } ;


enum TokenType {
  LPAREN = 1067, RPAREN = 2134, INT = 1164, STRING = 1358, DOT = 3201,
  FLOAT = 1552, NIL = 1261, T = 2522, QUOTE = 4268, SYMBOL = 1746
} ;

enum NodeType { EMPTY = 0, ATOM = 1, CONS = 2, SPECIAL = 3, ERROR = 4 } ;

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

class Exception {
public:
  string Err_mesg() {
    string mesg = "ERROR : " ;
    return mesg ;
  } // Err_mesg()
} ; // AssignedNotBoundException

class HighestException {
public:
  HighestException() {
    mMesg = "" ;
  } // HighestException()
  
  HighestException( string str ) {
    mMesg = str ;
  } // HighestException()
  
  string Err_mesg() {
    return mMesg ;
  } // Err_mesg()
  
protected:
  string mMesg ;
} ; // HighestException

class SingleList {
  
public:
  SingleList() {
    mRoot = NULL ;
    mTail = NULL ;
  } // SingleList()
  
  Node_Linear* mRoot ;
  Node_Linear* mTail ;
  
  // Purpose: Simply add a new node at the tail
  void AddNode( Token token ) {
    Node_Linear* newNode = new Node_Linear() ;
    newNode -> token = token ;
    newNode -> prev = NULL ;
    newNode -> next = NULL ;
    newNode -> isAddByMe = false ;
    
    if ( mRoot == NULL ) { // empty
      mRoot = newNode ;
      mTail = newNode ;
    } // if()
    else {
      if ( mTail != NULL && mTail == mRoot ) {
        mRoot -> next = newNode ;
        newNode -> prev = mRoot ;
        mTail = newNode ;
      } // if()
      else if ( mTail != NULL && mTail != mRoot ) {
        mTail -> next = newNode ;
        newNode -> prev = mTail ;
        mTail = mTail -> next ;
      } // else if()
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
    
    Node_Linear* newNode = new Node_Linear() ;
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
    
    if ( mRoot != NULL ) {
      
      while ( mRoot != NULL ) {
        Node_Linear* current = mRoot ;
        mRoot = mRoot -> next ;
        delete current ;
        current = NULL ;
      } // while()
      
      delete mRoot ;
      
      mRoot = NULL ;
      mTail = NULL ;
    } // if()
    else ;
    
  } // Clear()
  
} ;

static SingleList uOriginalList ;

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
    
    if ( str == "" ) {
      return false ;
    } // if()
    else {
      if ( str[ 0 ] == '+' || str[ 0 ] == '-' ) {
        startIndex = 1 ;  // the checking process start after the sign char
      } // if()
      
      for ( int i = startIndex ;  i < ( int ) str.length() ; i ++ ) {
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
    } // else()
    
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
    
    if ( str == "" ) {
      return false ;
    } // if()
    else {
      if ( str[ 0 ] == '+' || str[ 0 ] == '-' ) {
        startIndex = 1 ;  // the checking process start after the sign char
      } // if()
      
      for ( int i = startIndex ;  i < ( int ) str.length() ; i ++ ) {
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
    } // else()
    
    return true ;
  } // IsFLOAT()
  
  bool IsStr( string str ) {
    if ( str != "" && str[ 0 ] == '"' && str[ ( int ) str.length() - 1 ] == '"' ) {
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
      for ( int i = 0 ; i < RNUM ; i ++ ) {
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
  
  string FloatToStr( double num ) {
    string str = "" ;
    
    stringstream stream ;
    stream << fixed << setprecision( 3 ) << num ;
    
    str = stream.str() ;
    
    return str ;
  } // FloatToStr()
  
  int GetValueOfIntStr( string str ) {
    int num = 0 ;
    char sign = '\0' ;
    
    if ( str == "" ) {
      throw new Exception() ;
    } // if()
    else {
      if ( str[ 0 ] == '+' || str[ 0 ] == '-' ) {
        sign = str[ 0 ] ;
        str.erase( str.begin(), str.begin() + 1 ) ; // take off the sign char
      } // if()
      
      num = atoi( str.c_str() ) ;
      
      if ( sign == '-' ) {
        num *= -1 ;
      } // if()
    } // else()
    
    return num ;
  } // GetValueOfIntStr()
  
  double GetValueOfFloatStr( string str ) {
    double num = 0.0 ;
    char sign = '\0' ;
    
    if ( str == "" ) {
      throw new Exception() ;
    } // if()
    else {
      if ( str[ 0 ] == '+' || str[ 0 ] == '-' ) {
        sign = str[ 0 ] ;
        str.erase( str.begin(), str.begin() + 1 ) ; // take off the sign char
      } // if()
      
      num = atof( str.c_str() ) ;
      
      if ( sign == '-' ) {
        num *= -1.0 ;
      } // if()
    } // else()
    
    return num ;
  } // GetValueOfFloatStr()
  
  string FormatFloat( string str ) {
    string formatStr = "" ;
    
    if ( str != "" ) {
      
      if ( str[ ( int ) str.length() - 1 ] == '.' ) { // float num end with a dot
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
    } // if()
    else {
      throw new Exception() ;
    } // else()
    
    return formatStr ;
  } // FormatFloat()
  
  string FormatIntToFloatStr( string str ) {
    return str += ".000" ;
  } // FormatIntToFloatStr()
  
  Node* GetNullNode() {
    Node* null = new Node() ;
    null -> lex = "nil" ;
    null -> type = SPECIAL ;
    null -> parent = NULL ;
    null -> left = NULL ;
    null -> right = NULL ;
    null -> isAddByMe = false ;
    return null ;
  } // GetNullNode()
  
  void DeleteTree( Node* root ) {
    if ( root == NULL ) {
      return ;
    } // if()
    else if ( root -> right == NULL && root -> left == NULL ) {
      delete root ;
      root = NULL ;
      return ;
    } // else if()
    else {
      
      if ( root -> left != NULL ) {
        DeleteTree( root -> left ) ;
        root -> left = NULL ;
      } // if()
      else ;
      
      if ( root -> right != NULL ) {
        DeleteTree( root -> right ) ;
        root -> right = NULL ;
      } // if()
      else ;
      
    } // else()
    
    return ;
    
  } // DeleteTree()
  
  void Reset() {
    gLine = 1 ;
    gColumn = 0 ;
    uOriginalList.Clear() ;
    gPeekToken = "" ;
    gErrNode = NULL ;
  } // Reset()
  
  bool IsReturnLine( char ch ) {
    if ( ch == '\n' ) {
      return true ;
    } // if()
    
    return false ;
  } // IsReturnLine()
  
  void SkipLine() {
    char ch = '\0' ;
    ch = cin.peek() ;
    while ( ch != '\n' && !IsEOF( ch ) ) {
      ch = cin.get() ;
      ch = cin.peek() ;
    } // while()
    
  } // SkipLine()
  
  string GetStrContent( string str ) {
    string newString = "" ;
    
    if ( str != "" ) {
      if ( str[ 0 ] == '"' ) {
        newString = str.substr( 1, str.length() - 2 ) ;
      } // if()
      else {
        newString = str ;
      } // else()
    } // if()
    else {
      throw new Exception() ;
    } // else()
    
    return newString ;
  } // GetStrContent()
  
  void PrintStr( string str ) {
    
    for ( int i = 0 ; i < ( int ) str.length() ; i ++ ) {
      // this char is a '\\' and still has next char  behind
      if ( i < ( int ) str.length() - 1 && str[ i ] == '\\' ) {
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
    
    // cout << endl ;
  } // PrintStr()
  
  bool IsEOF( char ch ) {
    if ( ch == -1 ) { // -1 means -1 for cin.peek
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
        cout << "nil" ;
      } // if()
      else {
        cout << "#t" ;
      } // else()
    } // if()
    else if ( IsINT( r -> lex ) ) {
      cout << GetValueOfIntStr( r -> lex ) ;
    } // else if()
    else if ( IsFLOAT( r -> lex ) ) {
      cout << fixed << setprecision( 3 ) << GetValueOfFloatStr( r -> lex ) ;
    } // else if()
    else if ( IsStr( r -> lex ) ) {
      PrintStr( r -> lex ) ;
    } // else if()
    else cout << r -> lex ;
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
        cout << endl ;
      } // if()
      else {
        if ( r -> lex != "nil" && r -> lex != "#f" ) {
          PrintWhite( curLevel + 2 ) ;
          cout << "." << endl ;
          PrintWhite( curLevel + 2 ) ;
          PrettyPrintAtom( r ) ;
          cout << endl ;
        } // if()
        
        PrintWhite( curLevel ) ;
        cout << ")" ;
        
        if ( curLevel != 0 ) {
          cout << endl ;
        } // if()
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
          cout << endl ;
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
          cout << endl ;
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
    if ( r != NULL && r -> type != CONS ) { // this S-exp is an atom
      PrettyPrintAtom( r ) ;
      // cout << endl ;
    } // if()
    else if ( r != NULL && r -> type == CONS ) {
      PrettyPrintSExp( r, LEFT, 0, false ) ;
      // cout << endl ;
    } // else if()
    else ;
  } // PrettyPrint()
  
} ;

GlobalFunction gG ;

// --------------------- Error Definition Proj.1 (start) ---------------------

class MissingAtomOrLeftParException : public HighestException {
private:
  int mLine ;
  int mCol ;
  string mStr ;
  
public:
  MissingAtomOrLeftParException( int l, int c, string s ) {
    mLine = l ;
    mCol = c ;
    mStr = s ;
    
    mMesg = "ERROR (unexpected token) : atom or '(' expected when token at Line " + gG.IntToStr( mLine )
    + " Column " + gG.IntToStr( mCol - ( int ) mStr.length() + 1 ) + " is >>" + mStr + "<<" ;
    gG.SkipLine() ;
    // int tmpGLine = gLine ;
    gG.Reset() ;
    // gLine = tmpGLine ;
    
  } // MissingAtomOrLeftParException()
  
  string Err_mesg() {
    return mMesg ;
  } // Err_mesg()
} ; // MissingAtomOrLeftParException

class MissingRightParException : public HighestException {
private:
  int mLine ;
  int mCol ;
  string mStr ;
  
public:
  MissingRightParException( int l, int c, string s ) {
    mLine = l ;
    mCol = c ;
    mStr = s ;
    
    mMesg = "ERROR (unexpected token) : ')' expected when token at Line " + gG.IntToStr( mLine )
    + " Column " + gG.IntToStr( mCol + ( int ) mStr.length() - 1 ) + " is >>" + mStr + "<<" ;
    gG.SkipLine() ;
  } // MissingRightParException()
  
  string Err_mesg() {
    return mMesg ;
  } // Err_mesg()
} ; // MissingRightParException

class NoClosingQuoteException : public HighestException {
private:
  int mLine ;
  int mCol ;
  
public:
  NoClosingQuoteException( int l, int c ) {
    mLine = l ;
    mCol = c ;
    
    mMesg = "ERROR (no closing quote) : END-OF-LINE encountered at Line " + gG.IntToStr( mLine )
    + " Column " + gG.IntToStr( mCol ) ;
  } // NoClosingQuoteException()
  
  string Err_mesg() {
    return mMesg ;
  } // Err_mesg()
} ; // NoClosingQuoteException

class EOFException : public HighestException {
public:
  EOFException() {
    mMesg = "ERROR (no more input) : END-OF-FILE encountered" ;
    uOriginalList.Clear() ;
  } // EOFException()
  
  string Err_mesg() {
    return mMesg ;
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
    if ( ch == ' ' || ch == '\t' || IsReturnLine( ch ) ) {
      return true ;
    } // if()
    
    return false ;
  } // IsWhiteSpace()
  
  // Purpose: not only call the func. cin.get(), but also increase the column or line
  char GetChar() {
    char ch = '\0' ;
    ch = cin.get() ;
    if ( gG.IsEOF( ch ) ) {
      gIsEOF = true ;
      throw new EOFException() ;
    } // if()
    
    gColumn ++ ;
    
    return ch ;
  } // GetChar()
  
  string GetFullStr( string fullStr ) {
    // assert: 'ch' must be a peeked char which is '\"'
    bool keepRead = true ;
    char ch_get = '\0' ;
    char ch_peek = '\0' ;
    
    ch_peek = cin.peek() ;
    
    if ( ch_peek == '\"' && ( int ) fullStr.length() == 1 ) {
      ch_get = cin.get() ;
      fullStr += ch_get ;
      return fullStr ;
    } // if()
    // because we need to get a string, keep reading the input until
    // encounter the next '\"' or return-line
    
    try {
      
      while ( keepRead && !IsReturnLine( ch_peek ) && !gG.IsEOF( ch_peek ) ) {
        ch_get = cin.get() ;
        if ( gG.IsEOF( ch_get ) ) {
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
  
  void LexToToken( string lex, Token &token ) {
    token.str = "" ;
    token.line = 0 ;
    token.column = 0 ;
    if ( lex != "." && gG.IsFLOAT( lex ) ) {
      // assert: float with a original format
      // now  can start trandfer the float into the format which (int).(3 chars)
      token.str = gG.FormatFloat( lex ) ;
    } // if()
    else {
      token.str = lex ;
    } // else()
    
    token.line = gLine ;
    token.column = gColumn - ( int ) lex.length() + 1 ;
    token.type = gG.GetTokenType( lex ) ;
    
  } // LexToToken()
  
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
      
      // before get a actual char, we need to skip all the white-spaces first
      while ( IsWhiteSpace( ch ) || ch == ';' ) {
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
        else if ( ch == ';' ) { // is ';'
          // string tmpStr = "" ;
          // getline( cin, tmpStr ) ;
          gG.SkipLine() ; // didn't get '\n'
        } // else if()
        else ;
        
        ch = cin.peek() ;
      } // while()
      
      // assert: finally get a char which is not a white-space, now can start to construct a token
      ch = GetChar() ;  // since this char is not a white-space, we can get it from the input
      tokenStrWeGet += ch ;  // directly add the first non-white-space char into the token string
      
      // if this char is already a separator then STOP reading, or keep getting the next char
      if ( !IsSeparator( ch ) && ch != '\"' ) {  // 'ch' here is the first char overall
        ch = cin.peek() ;
        
        // check whether EOF because we may encounter EOF while making a peek token
        while ( !IsSeparator( ch ) && !IsWhiteSpace( ch ) && !gG.IsEOF( ch ) ) {
          ch = GetChar() ;
          tokenStrWeGet += ch ;
          ch = cin.peek() ;
        } // while()
        
      } // if()
      else if ( ch == '\"' ) {
        // assert: we get the whole token
        tokenStrWeGet = GetFullStr( tokenStrWeGet ) ;
      } // else if()
      else ;
      
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
    
    Token tokenWeWant ;
    LexToToken( gPeekToken, tokenWeWant ) ;
    gPeekToken = "" ;
    uOriginalList.AddNode( tokenWeWant ) ;
    
    return tokenWeWant ;
  } // GetToken()
  
  
};

LexicalAnalyzer gLA ;

// Purpose: Check the statement, if nothin wrong the build the tree, else Print the error
class SyntaxAnalyzer {

public:
  
  bool CheckSExp( Token startToken ) {
    // assert: startToken can only has three possibility
    // 1.Atom 2.LP 3.Quote *4.LR RP
    
    // this is a NIL with special format >()<
    if ( startToken.type == LPAREN && gG.GetTokenType( gLA.PeekToken() ) == RPAREN ) {
      gLA.GetToken() ; // take away the RP from the input
      
      return true ; // one of an ATOM
    } // if()
    else if ( IsATOM( startToken ) ) {
      return true ;
    } // else if()
    else if ( startToken.type == QUOTE ) {
      Token token = gLA.GetToken() ; // get the next token, suppose to be the start of a S-exp
      
      return CheckSExp( token ) ;
    } // else if()
    else if ( startToken.type == LPAREN ) {
      // suppose to have at least ONE S-exp
      bool hasOneSExpCorrect = false ;
      bool moreSExpCorrect = true ;
      
      Token token = gLA.GetToken() ; // get the next token, suppose to be the start of a S-exp
      hasOneSExpCorrect = CheckSExp( token ) ;
      
      if ( hasOneSExpCorrect ) {
        
        while ( ( gG.GetTokenType( gLA.PeekToken() ) == LPAREN
                  || IsATOM( gLA.PeekToken() )
                  || gG.GetTokenType( gLA.PeekToken() ) == QUOTE )
                && moreSExpCorrect ) {
          token = gLA.GetToken() ;
          moreSExpCorrect = CheckSExp( token ) ;
          
          gLA.PeekToken() ; // maybe successfully check a correct S-exp, keep peeking the next one
        } // while()
        
        if ( !moreSExpCorrect ) { // there are more S-exp, but not all correct
          return false ;
        } // if()
        else {
          // means only one S-exp in this left S-exp
          if ( gG.GetTokenType( gLA.PeekToken() ) == DOT ) {
            token = gLA.GetToken() ; // must be DOT
            // must be the start of the next S-exp according to the grammer
            token = gLA.GetToken() ;
            
            hasOneSExpCorrect = CheckSExp( token ) ;
            if ( !hasOneSExpCorrect ) {
              throw new MissingAtomOrLeftParException( gLine, gColumn, gPeekToken ) ;
              return false ;
            } // if()
            else {
              if ( gG.GetTokenType( gLA.PeekToken() ) == RPAREN ) {
                token = gLA.GetToken() ; // must be >)<
                
                return true ;
              } // if()
              else {
                throw new MissingRightParException( gLine, gColumn, gPeekToken ) ;
                return false ;
              } // else()
            } // else()
            
          } // if()
          else if ( gG.GetTokenType( gLA.PeekToken() ) == RPAREN ) {
            token = gLA.GetToken() ; // must be >)<
            
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
    TokenType type = gG.GetTokenType( str ) ;
    if ( type == SYMBOL || type == INT || type == FLOAT || type == STRING || type == NIL || type == T ) {
      return true ;
    } // if()
    
    return false ;
  } // IsATOM()
  
} ;

SyntaxAnalyzer gSA ;

class Tree {
  
private:
  
  Node* mRoot ;
  // SingleList mCopyList ;
  
  void TransferNIL( Node_Linear* root, Node_Linear* tail ) {
    
    // only ()
    if ( root -> token.type == LPAREN && root -> next -> token.type == RPAREN ) {
      Node_Linear* nilNode = new Node_Linear() ;
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
          walk -> next != NULL && walk -> next -> next != NULL ;
          walk = walk -> next ) {
      
      if ( walk -> next -> token.type == LPAREN && walk -> next -> next -> token.type == RPAREN ) {
        Node_Linear* nilNode = new Node_Linear() ;
        nilNode -> prev = NULL ;
        nilNode -> next = NULL ;
        nilNode -> token.str = "nil" ;
        nilNode -> token.type = NIL ;
        nilNode -> token.line = walk -> next -> token.line ;
        nilNode -> token.column = walk -> next -> token.column ;
        nilNode -> isAddByMe = false ;
        
        nilNode -> next = walk -> next -> next -> next ;
        
        if ( walk -> next -> next -> next != NULL ) {
          walk -> next -> next -> next -> prev = nilNode ;
        } // if()
        else ;
        
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
    
    Node_Linear* walk = NULL ;
    for ( walk = tail -> prev ; walk != NULL && walk != root ; ) {
      if ( walk -> token.type == LPAREN ) {
        while ( !s.empty() && s.top() -> token.type != RPAREN ) {
          if ( s.top() -> token.type == DOT ) count -- ;
          s.pop() ;
        } // while()
        
        if ( !s.empty() ) {
          s.pop() ; // last right par
        } // if()
      } // if()
      else {
        if ( walk != NULL && walk -> token.type == DOT ) count ++ ;
        s.push( walk ) ;
      } // else()
      
      if ( walk != root ) walk = walk -> prev ;
      
    } // for()
    
    if ( count == 1 ) {
      while ( !s.empty() && s.top() -> token.type != DOT ) { // right part of the cons is a list
        s.pop() ;
      } // while()
      
      return s.top() ;
    } // if()
    
    return NULL ;
    
  } // FindDOT()
  
  // Purpose: focus on one S-exp and give it the parathesis
  // Only list can call this function
  void Translate( Node_Linear* root, Node_Linear* tail ) {
    
    if ( uOriginalList.mRoot -> token.type == LPAREN
         && uOriginalList.mRoot -> next -> token.type == RPAREN
         && uOriginalList.mRoot -> next -> next == NULL ) {
      
      Node_Linear* nilNode = new Node_Linear() ;
      nilNode -> prev = NULL ;
      nilNode -> next = NULL ;
      nilNode -> token.str = "nil" ;
      nilNode -> token.type = NIL ;
      nilNode -> token.line = uOriginalList.mRoot -> token.line ;
      nilNode -> token.column = uOriginalList.mRoot -> token.column ;
      nilNode -> next = NULL ;
      nilNode -> isAddByMe = false ;
      
      while ( uOriginalList.mRoot != NULL ) { // Clear ( )
        Node_Linear* current = uOriginalList.mRoot ;
        uOriginalList.mRoot = uOriginalList.mRoot -> next ;
        delete current ;
        current = NULL ;
      } // while()
      
      uOriginalList.mRoot = nilNode ; // connect nil node
      uOriginalList.mTail = uOriginalList.mRoot ;
      return ;
    } // if()
    else {
      TransferNIL( root, tail ) ; // put a NIL in this list if needed
    } // else()
    
    int countPar = 0 ; // increase when manually add DOT and Paranthesis
    
    Node_Linear* dotPointer = FindDOT( root, tail ) ;
    if ( dotPointer == NULL ) { // there is no DOT, so put it on manually
      // assert: there is no DOT so need to add DOT and nil
      uOriginalList.InsertNode( tail -> prev, DOT ) ;
      uOriginalList.InsertNode( tail -> prev, NIL ) ;
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
        uOriginalList.InsertNode( walk, DOT ) ;
        walk = walk -> next ;
        uOriginalList.InsertNode( walk, LPAREN ) ;
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
            uOriginalList.InsertNode( walk, RPAREN ) ;
          } // for()
        } // if()
        else { // only an atom
          walk = walk -> next ; // skip DOT
          for ( int i = 0 ; i < countPar ; i ++ ) {
            uOriginalList.InsertNode( walk, RPAREN ) ;
          } // for()
        } // else()
        
        hasFinish = true ;
      } // else()
      
      if ( walk != NULL ) walk = walk -> next ;
    } // for()
    
  } // Translate()
  
  void TranslateQuote() { // process mCopyList
    // from back to front
    for ( Node_Linear* walk = uOriginalList.mTail ; walk != NULL ; walk = walk -> prev ) {
      if ( walk -> token.str == "'" ) { // should make the transfer
        if ( walk -> next != NULL && walk -> next -> token.type == LPAREN ) {
          Node_Linear* corRightPar = FindCorrespondPar( walk -> next ) ;
          walk -> token.str = "quote" ;
          uOriginalList.InsertNode( walk -> prev, LPAREN ) ;
          uOriginalList.InsertNode( corRightPar, RPAREN ) ;
        } // if()
        else { // is a aimple symbol
          // uOriginalList.InsertNode( mCopyList.mRoot , QUOTE ) ;
          walk -> token.str = "quote" ;
          uOriginalList.InsertNode( walk -> prev, LPAREN ) ;
          uOriginalList.InsertNode( walk -> next, RPAREN ) ;
        } // else()
      } // if()
    } // for()
  } // TranslateQuote()
  
public:
  Tree() {
    mRoot = NULL ;
  } // Tree()
  
  // Purpose: Transfer the DS from list to pointer (tree)
  // Pre-request: tokens in vector construct a S-exp with correct grammer
  // Return the root of this tree
  void Build( Node_Linear* leftPointer, Node_Linear* rightPointer, Node* parent, string dir ) {
    
    // left part if the cons
    if ( leftPointer -> token.type == LPAREN && rightPointer -> token.type == DOT ) {
      
      parent -> lex = leftPointer -> next -> token.str ;
      if ( leftPointer -> next -> token.type == NIL || leftPointer -> next -> token.type == T ) {
        parent -> type = SPECIAL ;
        
        if ( leftPointer -> next -> isAddByMe ) {
          parent -> isAddByMe = true ;
        } // if()
        else ;
      } // if()
      else {
        parent -> type = ATOM ;
      } // else()
    } // if()
    else if ( leftPointer == rightPointer ) { // right part of the cons
      
      parent -> lex = leftPointer -> token.str ;
      if ( leftPointer -> token.type == NIL || leftPointer -> token.type == T ) {
        parent -> type = SPECIAL ;
        
        if ( leftPointer -> isAddByMe ) {
          parent -> isAddByMe = true ;
        } // if()
        else ;
      } // if()
      else {
        parent -> type = ATOM ;
      } // else()
    } // else if()
    else { // Now is still in ( ) form
      Node_Linear* dotPointer = FindDOT( leftPointer, rightPointer ) ;
      
      parent -> type = CONS ;
      parent -> left = new Node() ;
      parent -> left -> lex = "" ;
      parent -> left -> left = NULL ;
      parent -> left -> right = NULL ;
      parent -> left -> parent = NULL ;
      parent -> left -> type = EMPTY ;
      parent -> left -> isAddByMe = false ;
      
      parent -> right = new Node() ;
      parent -> right -> lex = "" ;
      parent -> right -> left = NULL ;
      parent -> right -> right = NULL ;
      parent -> right -> parent = NULL ;
      parent -> right -> type = EMPTY ;
      parent -> right -> isAddByMe = false ;
      
      Build( leftPointer -> next, dotPointer -> prev, parent -> left, "left" ) ;
      parent -> left -> parent = parent ;
      Build( dotPointer -> next, rightPointer -> prev, parent -> right, "right" ) ;
      parent -> right -> parent = parent ;
      
    } // else()
    
  } // Build()
  
  void BuildTree() {
    TranslateQuote() ;
    
    // Substitude () with nil and put on the ( )
    if ( gSA.IsATOM( uOriginalList.mRoot -> token ) ) {
      Node* leaf = new Node() ;
      leaf -> lex = "" ;
      leaf -> left = NULL ;
      leaf -> right = NULL ;
      leaf -> parent = NULL ;
      leaf -> type = EMPTY ;
      leaf -> isAddByMe = false ;
      
      leaf -> lex = uOriginalList.mRoot -> token.str ;
      if ( uOriginalList.mRoot -> token.type == NIL || uOriginalList.mRoot -> token.type == T ) {
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
      if ( uOriginalList.mRoot -> token.type == LPAREN && uOriginalList.mRoot
          -> next -> token.str == "exit" && uOriginalList.mRoot -> next -> next -> token.type == RPAREN ) {
        gIsEOF = true ;
        return ;
      } // if()
      
      Translate( uOriginalList.mRoot, uOriginalList.mTail ) ;
      // mCopyList.PrintForward() ;
      
      mRoot = new Node() ;
      mRoot -> lex = "" ;
      mRoot -> left = NULL ;
      mRoot -> right = NULL ;
      mRoot -> parent = NULL ;
      mRoot -> type = EMPTY ;
      mRoot -> isAddByMe = false ;
      
      Build( uOriginalList.mRoot, uOriginalList.mTail, mRoot, "root" ) ;
      /*
       if ( mRoot -> type == CONS && mRoot -> left -> lex == "exit"
       && mRoot -> right -> lex == "nil" ) {
       gIsEOF = true ;
       } // if()
      */
      // uOriginalList.Print() ;
      // uOriginalList.PrintForward() ;
      // mCopyList.PrintBackforward() ;
    } // else()
  } // BuildTree()
  
  Node* GetRoot() {
    return mRoot ;
  } // GetRoot()
  
  void DeleteTree() {
    if ( mRoot != NULL ) {
      gG.DeleteTree( mRoot ) ;
      mRoot = NULL ;
    } // if()
  } // DeleteTree()
  
};

// --------------------- Error Definition Proj.2 (start) ---------------------

class NonListException : public HighestException {
public:
  NonListException() {
    mMesg = "ERROR (non-list) : " ;
  } // NonListException()
  
  string Err_mesg() {
    return mMesg ;
  } // Err_mesg()
} ; // NonListException

class IncorrectNumberArgumentException : public HighestException {
private:
  string mLex ;
public:
  IncorrectNumberArgumentException( string funcName ) {
    mLex = funcName ;
    
    mMesg = "ERROR (incorrect number of arguments) : " + mLex ;
  } // IncorrectNumberArgumentException()
  
  string Err_mesg() {
    return mMesg ;
  } // Err_mesg()
} ; // IncorrectNumberArgumentException

class IncorrectArgumentTypeException : public HighestException {
private:
  string mFuncName ;
  
public:
  IncorrectArgumentTypeException( string errFuncName ) {
    mFuncName = errFuncName ;
    mMesg = "ERROR (" + mFuncName + " with incorrect argument type) : " ;
  } // IncorrectArgumentTypeException()
  
  string Err_mesg() {
    return mMesg ;
  } // Err_mesg()
  
} ; // IncorrectArgumentTypeException

class ApplyNonFunctionException : public HighestException {
public:
  ApplyNonFunctionException() {
    mMesg = "ERROR (attempt to apply non-function) : " ;
  } // ApplyNonFunctionException()
  
  string Err_mesg() {
    return mMesg ;
  } // Err_mesg()
} ; // ApplyNonFunctionException

class NoReturnValueException : public HighestException {
public:
  NoReturnValueException() {
    mMesg = "ERROR (no return value) : " ;
  } // NoReturnValueException()
  
  string Err_mesg() {
    return mMesg ;
  } // Err_mesg()
  
} ; // NoReturnValueException

class UnboundValueException : public HighestException {
private:
  string mLex ;
  
public:
  UnboundValueException( string errLex ) {
    mLex = errLex ;
    
    mMesg = "ERROR (unbound symbol) : " + mLex ;
  } // UnboundValueException()
  
  string Err_mesg() {
    return mMesg ;
  } // Err_mesg()
} ; // UnboundValueException

class DivideByZeroException : public HighestException {
public:
  DivideByZeroException() {
    mMesg = "ERROR (division by zero) : /" ;
  } // DivideByZeroException()
  
  string Err_mesg() {
    return mMesg ;
  } // Err_mesg()
} ; // DivideByZeroException

class DefineFormatException : public HighestException {
public:
  DefineFormatException() {
    mMesg = "ERROR (DEFINE format) : " ;
  } // DefineFormatException()
  
  string Err_mesg() {
    return mMesg ;
  } // Err_mesg()
} ; // DefineFormatException

class CondFormatException : public HighestException {
public:
  CondFormatException() {
    mMesg = "ERROR (COND format) : " ;
  } // CondFormatException()
  
  string Err_mesg() {
    return mMesg ;
  } // Err_mesg()
} ; // CondFormatException

class LevelException : public HighestException {
private:
  string mLex ;
public:
  LevelException( string funcName ) {
    mLex = funcName ;
    mMesg = "ERROR (level of " + mLex + ")" ;
  } // LevelException()
  
  string Err_mesg() {
    return mMesg ;
  } // Err_mesg()
} ; // CleanLevelException()

// --------------------- Error Definition Proj.2 (end) ---------------------
// --------------------- Error Definition Proj.3 (start) ---------------------

class DefineFormatException2 : public HighestException {
public:
  DefineFormatException2() {
    mMesg = "ERROR (define format)" ;
  } // DefineFormatException2()
  
  string Err_mesg() {
    return mMesg ;
  } // Err_mesg()
} ; // DefineFormatException

class FormatException : public HighestException {
private:
  string mLex ;
public:
  FormatException( string funcName ) {
    mLex = funcName ;
    mMesg = "ERROR (" + mLex + " format) : " ;
  } // FormatException()
  
  string Err_mesg() {
    return mMesg ;
  } // Err_mesg()
} ; // DefineFormatException

class NonReturnAssignedException : public HighestException {
public:
  NonReturnAssignedException() {
    mMesg = "ERROR (no return value) : " ;
  } // NonReturnAssignedException()
  
  string Err_mesg() {
    return mMesg ;
  } // Err_mesg()
} ; // NonReturnAssignedException

class ParamNotBoundException : public HighestException {
public:
  ParamNotBoundException() {
    mMesg = "ERROR (unbound parameter) : " ;
  } // ParamNotBoundException()
  
  string Err_mesg() {
    return mMesg ;
  } // Err_mesg()
} ; // ParamNotBoundException

class TestCondNotBoundException : public HighestException {
public:
  TestCondNotBoundException() {
    mMesg = "ERROR (unbound test-condition) : " ;
  } // TestCondNotBoundException()
  
  string Err_mesg() {
    return mMesg ;
  } // Err_mesg()
} ; // TestCondNotBoundException

class CondNotBoundException : public HighestException {
public:
  CondNotBoundException() {
    mMesg = "ERROR (unbound condition) : " ;
  } // CondNotBoundException()
  
  string Err_mesg() {
    return mMesg ;
  } // Err_mesg()
} ; // CondNotBoundException

// --------------------- Error Definition Proj.3 (end) ---------------------

struct Symbol {
  string name ;
  Node* tree ;
} ; // SymbolInfo

struct LocalSymbol {
  Symbol* symbol ;
  int level ;
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
  StackArea* mStart ;
  StackArea* mEnd ;
  vector<LocalSymbol> mCallStack ; // the first element is the lastest one
  
public:
  vector<string> mCurrentVar ; // record the recent local variable (in same level)
  
  CallStack() {
    mStart = NULL ;
    mEnd = NULL ;
    mCurrentVar.clear() ;
    mCallStack.clear() ;
  } // CallStack()
  
  bool SymboIsInCallStack( string str ) {
    for ( int i = ( int ) mCallStack.size() - 1 ; i >= 0 ; i -- ) {
      if ( mCallStack.at( i ).symbol != NULL && str == mCallStack.at( i ).symbol -> name ) {
        return true ;
      } // if()
      else ;
    } // for()
    
    return false ;
  } // SymboIsInCallStack()
  
  void AddNewSymbolNameToStackArea() {
    if ( mStart == NULL ) {
      mStart = new StackArea() ;
      mStart -> currentArea.clear() ;
      mStart -> next = NULL ;
      mStart -> prev = NULL ;
      
      if ( !mCurrentVar.empty() ) {
        mStart -> currentArea.assign( mCurrentVar.begin(), mCurrentVar.end() ) ;
      } // if()
      else ; // the current stack area is empty
      
      mEnd = mStart ;
    } // if()
    else { // insert the new names
      mEnd -> next = new StackArea() ;
      mEnd -> next -> currentArea.clear() ;
      mEnd -> next -> next = NULL ;
      mEnd -> next -> prev = mEnd ;
      mEnd = mEnd -> next ;
      if ( !mCurrentVar.empty() ) {
        mEnd -> currentArea.assign( mCurrentVar.begin(), mCurrentVar.end() ) ;
      } // if()
      else ;
    } // else()
  } // AddNewSymbolNameToStackArea()
  
  void RestoreLocalVar( vector<Symbol> list ) {
    for ( int i = 0 ; i < ( int ) list.size() ; i ++ ) {
      mCurrentVar.push_back( list[ i ].name ) ;
    } // for()
  } // RestoreLocalVar()
  
  void RestoreLocalVar( vector<string> paramList ) {
    mCurrentVar.clear() ;
    mCurrentVar.assign( paramList.begin(), paramList.end() ) ;
  } // RestoreLocalVar()
  
  void DeleteLastStackArea() {
    if ( mEnd == mStart ) {
      if ( mEnd == NULL ) {
        return ;
      } // if()
      else {
        mStart -> currentArea.clear() ;
        delete mStart ;
        mStart = NULL ;
        mEnd = NULL ;
      } // else()
    } // if()
    else if ( mEnd -> prev != NULL ) {
      if ( mEnd -> prev == mStart ) {
        mEnd = mEnd -> prev ;
        mStart -> next -> currentArea.clear() ;
        delete mStart -> next ;
        mStart -> next = NULL ;
      } // if()
      else {
        mEnd = mEnd -> prev ;
        mEnd -> next -> currentArea.clear() ;
        delete mEnd -> next ;
        mEnd -> next = NULL ;
      } // else()
    } // else if()
  } // DeleteLastStackArea()
  
  void AddLocalVars( vector<string> nameList, vector<Node*> bindList, int level ) {
    if ( ( int ) nameList.size() != ( int ) bindList.size() ) {
      throw new Exception() ;
    } // if()
    else {
      for ( int i = 0 ; i < ( int ) nameList.size() && i < ( int ) bindList.size() ; i ++ ) {
        AddOneLocalVar( nameList[ i ], bindList[ i ], level ) ;
      } // for()
    } // else()
  } // AddLocalVars()
  
  void AddOneLocalVar( string name, Node* binding, int level ) {
    LocalSymbol newSym ;
    newSym.symbol = new Symbol() ;
    newSym.symbol -> name = "" ;
    newSym.level = 0 ;
    newSym.symbol -> tree = NULL ;
    
    newSym.symbol -> name = name ; // make a new symbol which is a local variable
    newSym.level = level ;
    newSym.symbol -> tree = binding ;
    
    mCurrentVar.push_back( name ) ;
    mCallStack.push_back( newSym ) ;
  } // AddOneLocalVar()
  
  void GetCleanLocalZone() {
    AddNewSymbolNameToStackArea() ;
    mCurrentVar.clear() ;
  } // GetCleanLocalZone()
  
  void ClearCurrentLocalVar() {
    
    int index = ( int ) mCallStack.size() - 1 ;
    for ( int i = ( int ) mCurrentVar.size() - 1 ; i >= 0 && index >= 0 ; i -- ) {
      if ( mCurrentVar.at( i ) == mCallStack.at( index ).symbol -> name ) {
        mCallStack.erase( mCallStack.begin() + index ) ;
      } // if()
      
      index -- ;
    } // for()
    
    mCurrentVar.clear() ;
    if ( mEnd != NULL ) {
      if ( ( int ) mEnd -> currentArea.size() > 0 ) {
        mCurrentVar.assign( mEnd -> currentArea.begin(), mEnd -> currentArea.end() ) ;
      } // if()
      else ;
      
      DeleteLastStackArea() ;
    } // if()
  } // ClearCurrentLocalVar()
  
  void RemoveLocalVarFromCurrentZone( vector<string> strList ) {
    for ( int i = ( int ) strList.size() - 1 ; i >= 0 ; i -- ) {
      bool hasRemoved = false ;
      
      for ( int j = ( int ) mCurrentVar.size() - 1 ; !hasRemoved && j >= 0 ; j -- ) {
        if ( strList[ i ] == mCurrentVar[ j ] ) {
          mCurrentVar.erase( mCurrentVar.begin() + j ) ;
          hasRemoved = true ;
        } // if()
        else ;
      } // for()
    } // for()
  } // RemoveLocalVarFromCurrentZone()
  
  void ClearRecentLocalVar( vector<string> strList ) {
    for ( int i = ( int ) strList.size() - 1 ; i >= 0 ; i -- ) {
      bool hasErased = false ;
      
      for ( int j = ( int ) mCurrentVar.size() - 1 ; !hasErased && j >= 0 ; j -- ) {
        if ( strList[ i ] == mCurrentVar[ j ] ) {
          int indexInStack = -1 ;
          indexInStack = GetLocalVarIndex( strList[ i ] ) ;
          if ( indexInStack != -1 && !mCallStack.empty() && !mCurrentVar.empty() ) {
            mCallStack.erase( mCallStack.begin() + indexInStack ) ;
            mCurrentVar.erase( mCurrentVar.begin() + j ) ;
          } // if()
          else ;
          
          hasErased = true ;
        } // if()
        else ;
      } // for()
    } // for()
  } // ClearRecentLocalVar()
  
  bool IsLocalVar( string name ) {
    bool isLocal = false ;
    for ( int i = 0 ; i < ( int ) mCurrentVar.size() && !isLocal ; i ++ ) {
      if ( name == mCurrentVar[ i ] ) {
        isLocal = true ;
      } // if()
    } // for()
    
    return isLocal ;
  } // IsLocalVar()
  
  int GetLocalVarIndex( string varName ) {
    for ( int i = ( int ) mCurrentVar.size() - 1 ; i >= 0 ; i -- ) {
      if ( varName == mCurrentVar[ i ] ) {
        for ( int j = ( int ) mCallStack.size() - 1 ; j >= 0 ; j -- ) {
          if ( varName == mCallStack.at( j ).symbol -> name ) {
            return j ;
          } // if()
        } // for()
      } // if()
    } // for()
    
    return -1 ;
  } // GetLocalVarIndex()
  
  Symbol* FindLocalSymbol( string symName ) {

    Symbol* target = NULL ;
    bool hasFind = false ;
    
    for ( int i = ( int ) mCurrentVar.size() - 1 ; target != NULL && i >= 0 ; i -- ) {
      if ( symName == mCurrentVar[ i ] ) {
        for ( int j = ( int ) mCallStack.size() - 1 ; j >= 0 && !hasFind ; j -- ) {
          if ( symName == mCallStack.at( j ).symbol -> name ) {
            target = mCallStack.at( j ).symbol ;
            hasFind = true ;
          } // if()
        } // for()
      } // if()
    } // for()
    
    return target ;
  } // FindLocalSymbol()
  
  Node* GetLocalVarBinding( string varName ) {
    if ( IsLocalVar( varName ) ) {
      int index = -1 ;
      index = GetLocalVarIndex( varName ) ;
      
      if ( index >= 0 && index < ( int ) mCallStack.size() ) {
        return mCallStack.at( index ).symbol -> tree ;
      } // if()
      else ;
    } // if()
    
    return NULL ;
  } // GetLocalVarBinding()
  
  void UpdateVar( string str, Node* newBinding ) {
    int index = -1 ;
    index = GetLocalVarIndex( str ) ;
    
    if ( index >= 0 && index < ( int ) mCallStack.size() ) {
      mCallStack.at( index ).symbol -> tree = newBinding ;
    } // if()
    else {
      throw new Exception() ;
    } // else()
  } // UpdateVar()
  
  void CleanStack() {
    mCurrentVar.clear() ;
    mCallStack.clear() ;
    
    if ( mStart != NULL ) {
      while ( mStart != NULL ) {
        StackArea* current = mStart ;
        mStart = mStart -> next ;
        delete current ;
        current = NULL ;
      } // while()
      
      delete mStart ;
      
      mStart = NULL ;
      mEnd = NULL ;
    } // if()
    else ;
    
    mStart = NULL ;
    mEnd = NULL ;
  } // CleanStack()
  
} ; // CallStack

struct ReserveWord {
  string name ;
  vector<string> list ;
} ; // ReserveWord

static Tree uTree ;

// Purpose: Do the evaluation and store the user definitions
class Evaluator {
private:
  
  vector<Symbol*> mSymbolTable ;
  vector<ReserveWord> mReserveWords ;
  CallStack mCallStack ;
  vector<Function*> mUserDefinedFunctionTable ;
  Function* mLambdaFunc ; // used to temporary store the lambda function
  vector<Function*> mLambdaStack ;
  
  void InitialReserveWord() {
    for ( int i = 0 ; i < RNUM ; i ++ ) {
      ReserveWord tmpWord ;
      tmpWord.name = "" ;
      tmpWord.name = gOriginReserveWordList[ i ] ;
      tmpWord.list.clear() ;
      mReserveWords.push_back( tmpWord ) ;
    } // for()
  } // InitialReserveWord()
  
  void ResetReserveWord() {
    for ( int i = 0 ; i < ( int ) mReserveWords.size() ; i ++ ) {
      mReserveWords.at( i ).list.clear() ;
    } // for()
  } // ResetReserveWord()
  
  void AddOriginReserveWords() {
    
    for ( int i = 0 ; i < RNUM ; i ++ ) {
      Node* tmpNode = new Node() ;
      tmpNode -> lex = "#<procedure " + gG.GetStrContent( gOriginReserveWordList[ i ] ) + ">" ;
      tmpNode -> type = ATOM ;
      tmpNode -> parent = NULL ;
      tmpNode -> left = NULL ;
      tmpNode -> right = NULL ;
      tmpNode -> isAddByMe = false ;
      
      Symbol* tmpSym = new Symbol() ;
      tmpSym -> name = gOriginReserveWordList[ i ] ;
      tmpSym -> tree = tmpNode ;
      
      mSymbolTable.push_back( tmpSym ) ;
    } // for()
  } // AddOriginReserveWords()
  
  void ResetSymbolTable() {
    if ( RNUM < ( int ) mSymbolTable.size() ) {
      mSymbolTable.erase( mSymbolTable.begin() + RNUM, mSymbolTable.end() ) ;
    } // if()
  } // ResetSymbolTable()
  
  Symbol* FindGlobalSymbol( string str ) {

    Symbol* target = NULL ;
    bool hasFound = false ;
    
    for ( int i = ( int ) mSymbolTable.size() - 1 ; i >= 0 && !hasFound ; i -- ) {
      if ( str == mSymbolTable.at( i ) -> name ) {
        target = mSymbolTable.at( i ) ;
        hasFound = true ;
      } // if()
      else ;
    } // for()
    
    return target ;
  } // FindGlobalSymbol()
  
  void UpdateGlobalSymbol( string name, Node* binding ) {
    bool finish = false ;
    for ( int i = ( int ) mSymbolTable.size() - 1 ; !finish && i >= 0 ; i -- ) {
      if ( name == mSymbolTable.at( i ) -> name ) {
        mSymbolTable.at( i ) -> tree = NULL ;
        mSymbolTable.at( i ) -> tree = binding ;

        finish = true ;
      } // if()
      else ;
    } // for()
  } // UpdateGlobalSymbol()
  
  bool IsGlobalSymbol( string str ) {
    if ( str == "" ) {
      return false ;
    } // if()
    else {
      for ( int i = ( int ) mSymbolTable.size() - 1 ; i >= 0 ; i -- ) {
        if ( mSymbolTable.at( i ) != NULL && str == mSymbolTable.at( i ) -> name ) {
          return true ;
        } // if()
        else ;
      } // for()
    } // else()
    
    return false ;
  } // IsGlobalSymbol()
  
  void CopyNode( Node* newNode, Node* oldNode ) {
    // Node* newN = new Node ;
    if ( newNode != NULL && oldNode != NULL ) {
      newNode -> lex = "" ;
      newNode -> type = EMPTY ;
      newNode -> left = NULL ;
      newNode -> right = NULL ;
      newNode -> isAddByMe = false ;
      newNode -> parent = NULL ;
      
      newNode -> lex = oldNode -> lex ;
      newNode -> type = oldNode -> type ;
      newNode -> left = oldNode -> left ;
      newNode -> right = oldNode -> right ;
      newNode -> isAddByMe = oldNode -> isAddByMe ;
      newNode -> parent = NULL ;
    } // if()
    else {
      throw new Exception() ;
    } // else()
    
    // return newN ;
  } // CopyNode()
  
  void CopyTree( Node* newTree, Node* oldTree, string dir ) {
    if ( oldTree -> type != CONS ) { // stop here
      if ( dir == "left" ) {
        CopyNode( newTree, oldTree ) ;
      } // if()
      else if ( dir == "right" ) {
        CopyNode( newTree, oldTree ) ;
      } // else if()
      else if ( dir == "root" ) {
        CopyNode( newTree, oldTree ) ;
      } // else if()
    } // if()
    else { // this is a cons
      newTree -> lex = "" ;
      newTree -> parent = NULL ;
      newTree -> type = CONS ;
      newTree -> isAddByMe = false ;
      newTree -> left = NULL ;
      newTree -> right = NULL ;
      
      newTree -> left = new Node() ;
      newTree -> left  -> lex = "" ;
      newTree -> left  -> parent = NULL ;
      newTree -> left  -> type = CONS ;
      newTree -> left  -> isAddByMe = false ;
      newTree -> left  -> left = NULL ;
      newTree -> left  -> right = NULL ;
      CopyTree( newTree -> left, oldTree -> left, "left" ) ;
      newTree -> left -> parent = newTree ;
      
      newTree -> right = new Node() ;
      newTree -> right -> lex = "" ;
      newTree -> right -> parent = NULL ;
      newTree -> right -> type = CONS ;
      newTree -> right -> isAddByMe = false ;
      newTree -> right -> left = NULL ;
      newTree -> right -> right = NULL ;
      CopyTree( newTree -> right, oldTree -> right, "right" ) ;
      newTree -> right -> parent = newTree ;
    } // else()
  } // CopyTree()
  
  void AddSymbol( Symbol* symbol ) {
    mSymbolTable.push_back( symbol ) ; // add this new symbol to the table
  } // AddSymbol()
  
  string GetReserveWordType( string str ) {
    if ( str == "" ) {
      return "" ;
    } // if()
    else {
      str = GetFuncNameFromFuncValue( str ) ;
      
      for ( int i = 0 ; i < ( int ) mReserveWords.size() ; i ++ ) {
        if ( str == mReserveWords.at( i ).name ) {
          return mReserveWords.at( i ).name ;
        } // if()
        
        for ( int j = 0 ; j < ( int ) mReserveWords[ i ].list.size() ; j ++ ) {
          if ( str == mReserveWords.at( i ).list[ j ] ) {
            return mReserveWords.at( i ).name ;
          } // if()
        } // for()
      } // for()
    } // else()
    
    return "" ;
  } // GetReserveWordType()
  
  string GetReserveWordType( string str, int &index ) {
    if ( str == "" ) {
      return "" ;
    } // if()
    else {
      str = GetFuncNameFromFuncValue( str ) ;
      
      for ( int i = 0 ; i < ( int ) mReserveWords.size() ; i ++ ) {
        if ( str == mReserveWords.at( i ).name ) {
          index = i ;
          return mReserveWords.at( i ).name ;
        } // if()
        
        for ( int j = 0 ; j < ( int ) mReserveWords[ i ].list.size() ; j ++ ) {
          if ( str == mReserveWords.at( i ).list[ j ] ) {
            index = i ;
            return mReserveWords.at( i ).name ;
          } // if()
        } // for()
      } // for()
    } // else()
    
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
  
  Symbol* FindSymbolFromLocalAndGlobal( string str ) {
    
    if ( mCallStack.IsLocalVar( str ) ) { // If this is a local variable, then find in stack
      return  mCallStack.FindLocalSymbol( str ) ;
    } // if()
    
    return FindGlobalSymbol( str ) ;
  } // FindSymbolFromLocalAndGlobal()
  
  bool SymbolExist( string str ) {
    if ( mCallStack.SymboIsInCallStack( str ) ) {
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
    else if ( IsGlobalSymbol( str ) ) {
      // not a local variable, now try to find in the global area
      bindResult = FindGlobalSymbol( str ) -> tree ;
    } // else if()
    else ;
    
    return bindResult ;
  } // FindSymbolBinding()
  
  int FindUserDefinedFunc( string str ) {
    int index = -1 ;
    
    if ( str == "" ) {
      return -1 ;
    } // if()
    else {
      str = GetFuncNameFromFuncValue( str ) ;
      
      for ( int i = ( int ) mUserDefinedFunctionTable.size() - 1 ; i >= 0 && index == -1 ; i -- ) {
        if ( str == mUserDefinedFunctionTable.at( i ) -> name ) {
          index = i ;
        } // if()
      } // for()
    } // else()
    
    return index ;
  } // FindUserDefinedFunc()
  
  bool IsList( Node* origin ) {
    Node* walk = NULL ;
    walk = origin ;
    if ( walk != NULL ) {
      for ( ; walk -> right != NULL ; walk = walk -> right ) { } ;
      
      if ( walk -> type != CONS && walk -> lex == "nil" ) {
        return true ;
      } // if()
      else ;
    } // if()
    else ;
    
    return false ;
  } // IsList()
  
  int CountListElement( Node* tree ) {
    int count = 0 ;
    for ( Node* walk = tree ; walk != NULL ; walk = walk -> right ) {
      if ( walk -> left != NULL ) {
        count ++ ;
      } // if()
    } // for()
    
    return count ;
  } // CountListElement()
  
  int CountArgument( Node* tree ) {
    int count = -1 ;
    for ( Node* walk = tree ; walk != NULL ; walk = walk -> right ) {
      if ( walk -> left != NULL ) {
        count ++ ;
      } // if()
    } // for()
    
    return count ;
  } // CountArgument()
  
  bool IsSymbol( string str ) {
    if ( str != "" && gG.GetTokenType( str ) == SYMBOL ) {
      return true ;
    } // if()
    
    return false ;
  } // IsSymbol()
  
  bool IsUserDefinedFunc( string str ) {
    for ( int i = 0 ; i < ( int ) mUserDefinedFunctionTable.size() ; i ++ ) {
      if ( str == mUserDefinedFunctionTable.at( i ) -> name ) {
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
      Node* firstArg = NULL ; // assume aug1 is a list
      Node* secondArg = NULL ; // assume aug2 is a list
      firstArg = inTree -> right -> left ; // assume aug1 is a list
      secondArg = inTree -> right -> right -> left ; // assume aug2 is a list
      
      consNode = new Node() ;
      consNode -> lex = "" ;
      consNode -> type = CONS ;
      consNode -> left = NULL ;
      consNode -> right = NULL ;
      consNode -> parent = NULL ;
      consNode -> isAddByMe = false ;
      
      if ( IsList( firstArg ) || firstArg -> type != CONS ) {
        if ( firstArg -> type == ATOM
             && IsSymbol( firstArg -> lex  )
             && !SymbolExist( firstArg -> lex ) ) {
          throw new UnboundValueException( firstArg -> lex ) ;
        } // if()
        else {
          Node* leftCons = NULL ;
          leftCons = EvaluateSExp( firstArg, level + 1 ) ;
          if ( !CheckHasReturnBinding( leftCons, firstArg ) ) {
            throw new ParamNotBoundException() ;
          } // if()
          else {
            consNode -> left = leftCons ;
          } // else()
          
          
          if ( IsList( secondArg ) || secondArg -> type != CONS ) {
            if ( secondArg -> type == ATOM
                 && IsSymbol( secondArg -> lex )
                 && !SymbolExist( secondArg -> lex ) ) {
              throw new UnboundValueException( secondArg -> lex ) ;
            } // if()
            else {
              // Step3. If no error create a new Node
              Node* rightCons = NULL ;
              rightCons = EvaluateSExp( secondArg, level + 1 ) ;
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
  
  void GetArgumentList( Node* inTree, vector<Node*> &newList ) {
    newList.clear() ;
    Node* walk = NULL ;
    
    if ( inTree != NULL ) {
      walk = inTree -> right ;
    } // if()
    else {
      return ;
    } // else()
    
    for ( ; walk != NULL && walk -> right != NULL ; walk = walk -> right ) {
      newList.push_back( walk -> left ) ;
    } // for()
    
  } // GetArgumentList()
  
  Node* EvaluateLIST( Node* inTree, int level ) {
    Node* result = NULL ;
    vector<Node*> argList ;
    if ( CountArgument( inTree ) > 0 ) {
      // Step1. find out all the arguments and put them in a list
      GetArgumentList( inTree, argList )  ;
      // Step2. start to check the type and mean time replace the symbol
      for ( int i = 0 ; i < ( int ) argList.size() ; i ++ ) {
        if ( IsList( argList[ i ] ) || argList[ i ] -> type != CONS ) {
          if ( argList[ i ] -> type == ATOM
               && IsSymbol( argList[ i ] -> lex ) ) {
            // int symIndex = FindSymbolFromLocalAndGlobal( argList[ i ] -> lex ) ;
            if ( SymbolExist( argList[ i ] -> lex ) ) {
              Node* result = NULL ;
              result = EvaluateSExp( argList[ i ], level + 1 ) ;
              CheckHasReturnBindingOrThrow( result, argList[ i ] ) ;
              // the result of this element in the list has a binding, keep going
              argList[ i ] = result ;
            } // if()
            else {
              throw new UnboundValueException( argList[ i ] -> lex ) ;
            } // else()
          } // if()
          else { // this is a list, but need to check more detail
            Node* result = NULL ;
            result = EvaluateSExp( argList[ i ], level + 1 ) ;
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
      for ( int i = 0 ; i < ( int ) argList.size() ; i ++ ) {
        Node* node = new Node() ;
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
        
        if ( i == ( int ) argList.size() - 1 ) {
          prevNode -> right = node ;
          node -> parent = prevNode ;
          
          Node* nilNode = new Node() ;
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
      
      argList.clear() ;
      
      return result ;
    } // if()
    else if ( CountArgument( inTree ) == 0 ) { // no element list
      return gG.GetNullNode() ;
    } // else if()
    else ;
    
    return NULL ; // it is empty in the argument
  } // EvaluateLIST()
  
  bool IsOriginalReserveWord( string name ) {
    for ( int i = 0 ; i < ( int ) mReserveWords.size() ; i ++ ) {
      if ( name == mReserveWords.at( i ).name ) {
        return true ;
      } // if()
    } // for()
    
    return false ;
  } // IsOriginalReserveWord()
  
  void EraseUserDefinedReserveWord( int index, string name ) {
    if ( index >= 0 && index < mReserveWords.size() ) {
      for ( int i = 0 ; i < ( int ) mReserveWords.at( index ).list.size() ; i ++ ) {
        if ( name == mReserveWords.at( index ).list[ i ] ) {
          mReserveWords.at( index ).list.erase( mReserveWords.at( index ).list.begin() + i ) ;
          return ;
        } // if()
      } // for()
    } // if()
  } // EraseUserDefinedReserveWord()
  
  void AddNewReserveWord( string reserveName, string newName ) {
    for ( int i = 0 ; i < RNUM ; i ++ ) {
      if ( reserveName == mReserveWords.at( i ).name ) {
        mReserveWords.at( i ).list.push_back( newName ) ;
        return ;
      } // if()
    } // for()
  } // AddNewReserveWord()
  
  void AddNewUserDefinedFunc( Function* func ) {
    /*
    Function tmpFunc ;
    tmpFunc.name = func.name ;
    tmpFunc.argNum = func.argNum ;
    tmpFunc.argList.assign( func.argList.begin(), func.argList.end() ) ;
    tmpFunc.tree = func.tree ;
    */
    
    mUserDefinedFunctionTable.push_back( func ) ;
  } // AddNewUserDefinedFunc()
  
  void UpdateUserDefinedFunc( Function* func ) {
    int funcIndex = -1 ;
    funcIndex = FindUserDefinedFunc( func -> name ) ;
    
    // this function is already exist
    if ( funcIndex >= 0 && funcIndex < ( int ) mUserDefinedFunctionTable.size() ) {
      /*
      mUserDefinedFunctionTable.at( funcIndex ).name = func.name ;
      mUserDefinedFunctionTable.at( funcIndex ).argNum = func.argNum ;
      mUserDefinedFunctionTable.at( funcIndex ).argList.clear() ;
      mUserDefinedFunctionTable.at( funcIndex ).argList.assign( func.argList.begin(), func.argList.end() ) ;
      mUserDefinedFunctionTable.at( funcIndex ).tree = func.tree ;
      */
      mUserDefinedFunctionTable.at( funcIndex ) = func ;
    } // if()
    else {
      throw new Exception() ;
    } // else()
  } // UpdateUserDefinedFunc()
  
  void DeleteUserDefineFunc( string name ) {
    int funcIndex = -1 ;
    funcIndex = FindUserDefinedFunc( name ) ;
    
    if ( funcIndex != -1 && funcIndex < ( int ) mUserDefinedFunctionTable.size() ) {
      mUserDefinedFunctionTable.erase( mUserDefinedFunctionTable.begin() + funcIndex ) ;
    } // if()
    else ;
  } // DeleteUserDefineFunc()
  
  bool IsALambdaFunc( Node* tree ) {
    if ( tree != NULL && tree -> left != NULL && tree -> left -> left != NULL
         && tree -> left -> left -> lex == "lambda" ) {
      return true ;
    } // if()
    
    return false ;
  } // IsALambdaFunc()
  
  Node* DefineUserFunc( Node* inTree, int level, bool isSet ) {
    Node* funcNameAndArgPart = NULL ;
    Node* procedurePart = NULL ;
    
    if ( CountArgument( inTree ) >= 2 ) {
      string funcName = "" ;
      Node* argList = NULL ;
      
      funcNameAndArgPart = inTree -> right -> left ;
      procedurePart = inTree -> right -> right ;
      
      if ( funcNameAndArgPart != NULL ) {
        funcName = funcNameAndArgPart -> left -> lex ;
        argList = funcNameAndArgPart -> right ;
        
        // if the function name is not a symbol and the binding has more than one
        if ( gG.IsSymbol( funcName ) && procedurePart != NULL ) {
          
          Function* newFunc = new Function() ;
          
          newFunc -> name = funcName ;
          newFunc -> argNum = CountAndCkeckParameters( argList, newFunc -> argList, "define" ) ;
          newFunc -> tree = procedurePart ;
          
          Node* tmp = new Node() ;
          tmp -> lex = "" ;
          tmp -> type = ATOM ;
          tmp -> left = NULL ;
          tmp -> right = NULL ;
          tmp -> parent = NULL ;
          tmp -> isAddByMe = false ;
          if ( mLambdaFunc -> tree != NULL ) {
            tmp -> lex = "#<procedure lambda>" ;
          } // if()
          else {
            tmp -> lex = "#<procedure " + funcName + ">" ;
          } // else()
          
          if ( !IsGlobalSymbol( funcName ) ) {
            Symbol* newSym = new Symbol() ;
            newSym -> name = funcName ;
            newSym -> tree = tmp ;
            AddSymbol( newSym ) ;
          } // if()
          else {
            UpdateGlobalSymbol( funcName, tmp ) ;
          } // else()
          
          if ( !IsUserDefinedFunc( funcName ) ) {
            AddNewUserDefinedFunc( newFunc ) ;
          } // if()
          else {
            UpdateUserDefinedFunc( newFunc ) ;
          } // else()
          
          if ( gVerbose && !isSet ) {
            cout << funcName << " defined" << endl ;
          } // if()
        } // if()
        else {
          gErrNode = inTree ;
          throw new DefineFormatException() ;
        } // else()
      } // if()
      else {
        gErrNode = inTree ;
        throw new DefineFormatException() ;
      } // else()
    } // if()
    else {
      gErrNode = inTree ;
      throw new DefineFormatException() ;
    } // else()
    
    return NULL ;
  } // DefineUserFunc()
  
  bool IsQuoteExp( Node* tree ) {
    if ( tree != NULL && tree -> type == CONS && tree -> left -> lex == "quote" ) {
      return true ;
    } // if()
    
    return false ;
  } // IsQuoteExp()
  
  Node* Define( Node* inTree, int level, bool isSet ) {
    int argNum = 0 ;
    // vector<Node*> argList ;
    argNum = CountArgument( inTree ) ;
    
    if ( level == 1 || isSet ) {
      Node* funcNameAndParamPart = NULL ;
      Node* statementPart = NULL ;
      
      funcNameAndParamPart = inTree -> right -> left ;
      statementPart = inTree -> right -> right ;
      // GetArgumentList( inTree, argList ) ;
      
      // the first argument should be a symbol
      if ( argNum == 2 && funcNameAndParamPart != NULL && funcNameAndParamPart -> type == ATOM ) {
        Symbol* newSymbol = new Symbol() ;
        newSymbol -> name = "" ;
        newSymbol -> tree = NULL ;
        
        string symName = funcNameAndParamPart -> lex ;
        
        // because this is a symbol definition, so only a S-exp
        statementPart = statementPart -> left ;
        
        int reserveIndex = -1 ;
        string reserveName = "" ;
        
        reserveName = GetReserveWordType( symName, reserveIndex ) ;
        
        if ( reserveName != "" && !IsOriginalReserveWord( symName ) ) {
          // may be a user new defined reverse word
          EraseUserDefinedReserveWord( reserveIndex, symName ) ;
          reserveName = "" ;
          
        } // if()
        
        if ( reserveName == "" ) {
          if ( gG.IsSymbol( symName ) ) {
            
            // int symIndex = FindSymbolFromLocalAndGlobal( argList[ 0 ] -> lex ) ;
            newSymbol -> name = symName ;
            // check the be binded s-exp is correct
            
            Node* bind = NULL ;
            // the definition of this symbol, need to check whether the binding exist
            // if not, then this is a non return value error
            bind = EvaluateSExp( statementPart, level + 1 ) ;
            CheckHasReturnBindingOrThrow( bind, statementPart ) ;
            
            newSymbol -> tree = bind ;
            
            if ( SymbolExist( symName ) ) { // this symbol has already exist, update it
              
              if ( !SymbolExist( statementPart -> lex ) ) {
                // the reference value is not a symbol
                if ( mCallStack.IsLocalVar( symName ) ) {
                  mCallStack.UpdateVar( symName, bind ) ;
                  
                } // if()
                else { // this new symbol should be a global symbol
                  UpdateGlobalSymbol( symName, bind ) ;
                  
                  if ( IsUserDefinedFunc( symName ) ) {
                    
                    // this symbol name has already used to define a function
                    // but now this symbol is used to a new binding which is not a function
                    // so need to remove this symbol name from the UserDefinedFuncTable
                    DeleteUserDefineFunc( symName ) ;
                  } // if()
                  else ;
                  
                  if ( newSymbol -> tree -> type != CONS
                       && newSymbol -> tree -> lex == "#<procedure lambda>" ) {
                    mLambdaFunc -> name = symName ;
                    AddNewUserDefinedFunc( mLambdaFunc ) ;
                  } // if()
                  
                  string tmpReserve = GetReserveWordType( GetFuncNameFromFuncValue( bind -> lex ) ) ;
                  if ( tmpReserve != "" ) {
                    AddNewReserveWord( tmpReserve, symName ) ;
                  } // if()
                } // else()
              } // if()
              else {
                string tmpReserve = GetReserveWordType( statementPart -> lex ) ;
                if ( tmpReserve != "" ) {
                  AddNewReserveWord( tmpReserve, symName ) ;
                } // if()
                
                UpdateGlobalSymbol( symName, bind ) ;
              } // else()
            } // if()
            else {
              
              if ( newSymbol -> tree != NULL && newSymbol -> tree -> lex != "#<procedure lambda>" ) {
                string reserveName = GetReserveWordType( newSymbol -> tree -> lex ) ;
                
                if ( reserveName != "" ) {
                  // this is a special case, define your own reserve word
                  // add this to the reserveWordList
                  AddNewReserveWord( reserveName, newSymbol -> name ) ;
                } // if()
              } // if()
              else if ( IsUserDefinedFunc( statementPart -> lex ) ) {
                Function* tmpFunc =
                mUserDefinedFunctionTable.at( FindUserDefinedFunc( statementPart -> lex ) ) ;
                
                AddNewUserDefinedFunc( tmpFunc ) ;
              } // else if()
              else if ( newSymbol -> tree != NULL && newSymbol -> tree -> lex == "#<procedure lambda>" ) {
                mLambdaFunc -> name = symName ;
                AddNewUserDefinedFunc( mLambdaFunc ) ;
              } // else if()
              else ;
              
              AddSymbol( newSymbol ) ;
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
        
        if ( gVerbose && !isSet ) {
          cout << symName << " defined" << endl ;
        } // if()
        
        return NULL ;
      } // if()
      else if ( argNum >= 2 && funcNameAndParamPart != NULL
                && funcNameAndParamPart -> type == CONS ) {
        
        DefineUserFunc( inTree, level, isSet ) ;
        
      } // else if()
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
    if ( CountArgument( inTree ) == 1 && inTree -> right != NULL ) {
      Node* targetTree = NULL ;
      
      if ( inTree -> right != NULL ) {
        targetTree = EvaluateSExp( inTree -> right -> left, level + 1 ) ;
      } // if()
      else ;
      
      if ( targetTree != NULL ) {
        if ( targetTree -> type == ATOM || targetTree -> type == SPECIAL ) {
          gErrNode = targetTree ;
          throw new IncorrectArgumentTypeException( funcName ) ;
        } // if()
        else if ( targetTree -> type == CONS ) {
          if ( funcName == "car" ) {
            return targetTree -> left ;
          } // if()
          else if ( funcName == "cdr" ) {
            return targetTree -> right ;
          } // else if()
        } // else if()
        else ;
      } // if()
      else {
        if ( inTree != NULL && inTree -> right != NULL ) {
          gErrNode = inTree -> right -> left ;
        } // if()
        else {
          gErrNode = NULL ;
        } // else()
        
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
    Node* ansNode = new Node() ;
    ansNode -> lex = "#t" ;
    ansNode -> type = SPECIAL ;
    ansNode -> parent = NULL ;
    ansNode -> left = NULL ;
    ansNode -> right = NULL ;
    ansNode -> isAddByMe = false ;
    // can only have ONE argement
    if ( CountArgument( inTree ) == 1 ) {
      Node* target = NULL ;
      if ( inTree -> right != NULL ) {
        target = EvaluateSExp( inTree -> right -> left, level + 1 ) ;
      } // if()
      else ;
      
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
          if ( target -> type == ATOM && gG.IsINT( target -> lex ) ) {
            ans = true ;
          } // if()
        } // else if()
        else if ( func == "real?" || func == "number?" ) {
          if ( target -> type == ATOM && ( gG.IsINT( target -> lex ) || gG.IsFLOAT( target -> lex ) ) ) {
            ans = true ;
          } // if()
        } // else if()
        else if ( func == "string?" ) {
          if ( target -> type == ATOM && gG.IsStr( target -> lex ) ) {
            ans = true ;
          } // if()
        } // else if()
        else if ( func == "boolean?" ) {
          if ( target -> type == SPECIAL ) {
            ans = true ;
          } // if()
        } // else if()
        else if ( func == "symbol?" ) {
          if ( target -> type == ATOM && gG.GetTokenType( target -> lex ) == SYMBOL ) {
            ans = true ;
          } // if()
        } // else if()
      } // if()
      else {
        if ( inTree != NULL && inTree -> right != NULL ) {
          gErrNode = inTree -> right -> left ;
        } // if()
        else {
          gErrNode = NULL ;
        } // else()
        
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
    Node* ansNode = new Node() ;
    ansNode -> lex = "" ;
    ansNode -> type = ATOM ;
    ansNode -> parent = NULL ;
    ansNode -> left = NULL ;
    ansNode -> right = NULL ;
    ansNode -> isAddByMe = false ;
    
    // check whether all the arguments are numbers
    for ( int i = 0 ; i < ( int ) argList.size() ; i ++ ) {
      Node* currentAug = NULL ;
      currentAug = EvaluateSExp( argList[ i ], level + 1 ) ;
      if ( currentAug != NULL && currentAug -> type == ATOM
           && ( gG.IsINT( currentAug -> lex )
                || gG.IsFLOAT( currentAug -> lex ) ) ) {
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
    
    if ( gG.IsINT( argList[ 0 ] -> lex ) ) {
      ans = gG.GetValueOfIntStr( argList[ 0 ] -> lex ) ;
    } // if()
    else if ( gG.IsFLOAT( argList[ 0 ] -> lex ) ) {
      hasFloatExist = true ;
      ans = gG.GetValueOfFloatStr( argList[ 0 ] -> lex ) ;
    } // else if()
    
    for ( int i = 1 ; i < ( int ) argList.size() ; i ++ ) {
      if ( gG.IsINT( argList[ i ] -> lex ) ) {
        if ( funcName == "+" ) {
          ans += gG.GetValueOfIntStr( argList[ i ] -> lex ) ;
        } // if()
        else if ( funcName == "-" ) {
          ans -= gG.GetValueOfIntStr( argList[ i ] -> lex ) ;
        } // else if()
        else if ( funcName == "*" ) {
          ans *= gG.GetValueOfIntStr( argList[ i ] -> lex ) ;
        } // else if()
        else if ( funcName == "/" ) {
          if ( gG.GetValueOfIntStr( argList[ i ] -> lex ) != 0 ) {
            ans /= gG.GetValueOfIntStr( argList[ i ] -> lex ) ;
          } // if()
          else {
            throw new DivideByZeroException() ;
          } // else()
        } // else if()
      } // if()
      else if ( gG.IsFLOAT( argList[ i ] -> lex ) ) {
        hasFloatExist = true ;
        if ( funcName == "+" ) {
          ans += gG.GetValueOfFloatStr( argList[ i ] -> lex ) ;
        } // if()
        else if ( funcName == "-" ) {
          ans -= gG.GetValueOfFloatStr( argList[ i ] -> lex ) ;
        } // else if()
        else if ( funcName == "*" ) {
          ans *= gG.GetValueOfFloatStr( argList[ i ] -> lex ) ;
        } // else if()
        else if ( funcName == "/" ) {
          if ( gG.GetValueOfFloatStr( argList[ i ] -> lex ) != 0 ) {
            ans /= gG.GetValueOfFloatStr( argList[ i ] -> lex ) ;
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
      ansNode -> lex = gG.IntToStr( intAns ) ;
    } // if()
    else { // the final answer is a float
      stringstream sstream ;
      sstream << ans ;
      
      // ansNode -> lex = to_string( ans ) ;
      sstream >> ansNode -> lex ;
      if ( gG.IsINT( ansNode -> lex ) ) {
        ansNode -> lex = gG.FormatIntToFloatStr( ansNode -> lex ) ;
      } // if()
      else {
        ansNode -> lex = gG.FormatFloat( ansNode -> lex ) ;
      } // else()
    } // else()
    
    return ansNode ;
  } // ProcessMath()
  
  Node* ProcessCompare( string funcName, vector<Node*> argList, int level ) {
    Node* ansNode = new Node() ;
    ansNode -> lex = "#t" ;
    ansNode -> type = SPECIAL ;
    ansNode -> parent = NULL ;
    ansNode -> left = NULL ;
    ansNode -> right = NULL ;
    ansNode -> isAddByMe = false ;
    
    // check whether all the arguments are numbers
    for ( int i = 0 ; i < ( int ) argList.size() ; i ++ ) {
      Node* currentAug = NULL ;
      currentAug = EvaluateSExp( argList[ i ], level + 1 ) ;
      if ( currentAug != NULL && currentAug -> type == ATOM
           && ( gG.IsINT( currentAug -> lex )
                || gG.IsFLOAT( currentAug -> lex ) ) ) {
        argList[ i ] = EvaluateSExp( argList[ i ], level + 1 ) ;
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
    if ( gG.IsINT( argList[ 0 ] -> lex ) ) {
      currentValue = gG.GetValueOfIntStr( argList[ 0 ] -> lex ) ;
    } // if()
    else if ( gG.IsFLOAT( argList[ 0 ] -> lex ) ) {
      currentValue = gG.GetValueOfFloatStr( argList[ 0 ] -> lex ) ;
    } // else if()
    
    for ( int i = 1 ; i < ( int ) argList.size() ; i ++ ) {
      double nextValue = 0.0 ;
      if ( gG.IsINT( argList[ i ] -> lex ) ) {
        nextValue = gG.GetValueOfIntStr( argList[ i ] -> lex ) ;
      } // if()
      else if ( gG.IsFLOAT( argList[ i ] -> lex ) ) {
        nextValue = gG.GetValueOfFloatStr( argList[ i ] -> lex ) ;
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
    Node* ansNode = new Node() ;
    ansNode -> lex = "#t" ;
    ansNode -> type = SPECIAL ;
    ansNode -> parent = NULL ;
    ansNode -> left = NULL ;
    ansNode -> right = NULL ;
    ansNode -> isAddByMe = false ;
    
    // check whether all the arguments are numbers
    for ( int i = 0 ; i < ( int ) argList.size() ; i ++ ) {
      Node* currentAug = NULL ;
      currentAug = EvaluateSExp( argList[ i ], level + 1 ) ;
      if ( currentAug != NULL
           && currentAug -> type == ATOM && gG.IsStr( currentAug -> lex ) ) {
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
    
    string currentStr = "" ;
    string ansStr = "" ; // only used in "string-append"
    
    currentStr = gG.GetStrContent( argList[ 0 ] -> lex ) ;
    ansStr = currentStr ; // only used in "string-append"
    
    for ( int i = 1 ; i < ( int ) argList.size() ; i ++ ) {
      if ( funcName == "string-append" ) {
        ansStr += gG.GetStrContent( argList[ i ] -> lex ) ;
      } // if()
      else if ( funcName == "string>?" ) {
        if ( currentStr <= gG.GetStrContent( argList[ i ] -> lex ) ) {
          ansNode -> lex = "nil" ;
          return ansNode ;
        } // if()
        else {
          currentStr = gG.GetStrContent( argList[ i ] -> lex ) ;
        } // else()
      } // else if()
      else if ( funcName == "string<?" ) {
        if ( currentStr >= gG.GetStrContent( argList[ i ] -> lex ) ) {
          ansNode -> lex = "nil" ;
          return ansNode ;
        } // if()
        else {
          currentStr = gG.GetStrContent( argList[ i ] -> lex ) ;
        } // else()
      } // else if()
      else if ( funcName == "string=?" ) {
        if ( currentStr != gG.GetStrContent( argList[ i ] -> lex ) ) {
          ansNode -> lex = "nil" ;
          return ansNode ;
        } // if()
        else {
          currentStr = gG.GetStrContent( argList[ i ] -> lex ) ;
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
    Node* ansNode = new Node() ;
    ansNode -> lex = "#t" ;
    ansNode -> type = SPECIAL ;
    ansNode -> parent = NULL ;
    ansNode -> left = NULL ;
    ansNode -> right = NULL ;
    ansNode -> isAddByMe = false ;
    
    bool resultIsTrue = true ;
    for ( int i = 0 ; i < ( int ) argList.size() ; i ++ ) {
      Node* currentAug = NULL ;
      currentAug = EvaluateSExp( argList[ i ], level + 1 ) ;
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
        if ( argList[ i ] -> type == SPECIAL
             && ( argList[ i ] -> lex != "#t" && argList[ i ] -> lex != "t" ) ) {
          resultIsTrue = false ;
          ansNode -> lex = "nil" ;
          return ansNode ;
        } // if()
        else if ( i == ( int ) argList.size() - 1 ) {
          return argList[ i ] ;
        } // else if()
      } // else if()
      else if ( funcName == "or" ) {
        resultIsTrue = false ;
        if ( argList[ i ] -> type != SPECIAL ) {
          return argList[ i ] ;
        } // if()
        else if ( argList[ i ] -> lex == "#t" || argList[ i ] -> lex == "t" ) {
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
    Node* ans = NULL ;
    vector<Node*> argList ;
    GetArgumentList( inTree, argList ) ;
    
    if ( IsMathOperator( funcName ) ) { // need to have more than two arguments
      if ( CountArgument( inTree ) >= 2 ) {
        ans = ProcessMath( funcName, argList, level + 1 ) ;
      } // if()
      else {
        argList.clear() ;
        throw new IncorrectNumberArgumentException( funcName ) ;
      } // else()
    } // if()
    else if ( IsComparison( funcName ) ) { // need to have more than two arguments
      if ( CountArgument( inTree ) >= 2 ) {
        ans = ProcessCompare( funcName, argList, level + 1 );
      } // if()
      else {
        argList.clear() ;
        throw new IncorrectNumberArgumentException( funcName ) ;
      } // else()
    } // else if()
    else if ( IsCondOperator( funcName ) ) { // not only need 1 argument
      if ( funcName == "not" ) { // only ONE argument
        if ( CountArgument( inTree ) == 1 ) {
          ans = ProcessCondOperation( funcName, argList, level + 1 ) ;
        } // if()
        else {
          argList.clear() ;
          throw new IncorrectNumberArgumentException( funcName ) ;
        } // else()
      } // if()
      else {
        if ( CountArgument( inTree ) >= 2 ) {
          ans = ProcessCondOperation( funcName, argList, level + 1 ) ;
        } // if()
        else {
          argList.clear() ;
          throw new IncorrectNumberArgumentException( funcName ) ;
        } // else()
      } // else()
    } // else if()
    else if ( IsStringOperator( funcName ) ) {
      // need to have more than two arguments
      if ( CountArgument( inTree ) >= 2 ) {
        ans = ProcessStringCompare( funcName, argList, level + 1 ) ;
      } // if()
      else {
        argList.clear() ;
        throw new IncorrectNumberArgumentException( funcName ) ;
      } // else()
    } // else if()
    
    argList.clear() ;
    
    return ans ;
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
    Node* ansNode = new Node() ;
    ansNode -> lex = "#t" ;
    ansNode -> type = SPECIAL ;
    ansNode -> parent = NULL ;
    ansNode -> left = NULL ;
    ansNode -> right = NULL ;
    ansNode -> isAddByMe = false ;
    
    if ( CountArgument( inTree ) == 2 ) {
      vector<Node*> argList ;
      GetArgumentList( inTree, argList ) ;
      
      for ( int i = 0 ; i < ( int ) argList.size() ; i ++ ) {
        EvaluateSExp( argList[ i ], level + 1 ) ; // only used to check whether is any wrong
      } // for()
      
      if ( funcName == "eqv?" ) { // compare the pointer
        bool isInSameMemory = false ;
        
        if ( argList[ 0 ] -> type != CONS && argList[ 1 ] -> type != CONS
             && gG.GetTokenType( argList[ 0 ] -> lex ) == SYMBOL
             && gG.GetTokenType( argList[ 1 ] -> lex ) == SYMBOL ) {
          // int symIndex1 = FindSymbolFromLocalAndGlobal( argList[ 0 ] -> lex ) ;
          // int symIndex2 = FindSymbolFromLocalAndGlobal( argList[ 1 ] -> lex ) ;
          string symName1 = "" ;
          string symName2 = "" ;
          Symbol sym1 ;
          sym1.name = "" ;
          sym1.tree = NULL ;
          Symbol sym2 ;
          sym2.name = "" ;
          sym2.tree = NULL ;
          
          symName1 = argList[ 0 ] -> lex ;
          symName2 = argList[ 1 ] -> lex ;
          
          if ( SymbolExist( symName1 ) && SymbolExist( symName2 ) ) {
            // case1. both of them are local
            if ( mCallStack.IsLocalVar( symName1 )
                 && mCallStack.IsLocalVar( symName2 ) ) {
              if ( mCallStack.GetLocalVarBinding( symName1 )
                   == mCallStack.GetLocalVarBinding( symName2 ) ) {
                isInSameMemory = true ;
              } // if()
            } // if()
            // case2. both of them are global
            else if ( !mCallStack.IsLocalVar( symName1 )
                      && !mCallStack.IsLocalVar( symName2 ) ) {
              if ( FindGlobalSymbol( symName1 ) -> tree == FindGlobalSymbol( symName2 ) -> tree ) {
                isInSameMemory = true ;
              } // if()
            } // else if()
          } // if()
        } // if()
        else if ( gG.IsINT( argList[ 0 ] -> lex )
                  && gG.IsINT( argList[ 1 ] -> lex ) ) {
          if ( gG.GetValueOfIntStr( argList[ 0 ] -> lex )
               == gG.GetValueOfIntStr( argList[ 1 ] -> lex ) ) {
            isInSameMemory = true ;
          } // if()
        } // else if()
        else if ( gG.IsFLOAT( argList[ 0 ] -> lex )
                  && gG.IsFLOAT( argList[ 1 ] -> lex ) ) {
          if ( gG.GetValueOfFloatStr( argList[ 0 ] -> lex )
               == gG.GetValueOfFloatStr( argList[ 1 ] -> lex ) ) {
            isInSameMemory = true ;
          } // if()
        } // else if()
        else { // check the #t and #f and nil and '()
          Node* tmp1 = NULL ;
          Node* tmp2 = NULL ;
          tmp1 = EvaluateSExp( argList[ 0 ], level + 1 ) ;
          tmp2 = EvaluateSExp( argList[ 1 ], level + 1 ) ;
          CheckHasRaramBindingOrThrow( tmp1, argList[ 0 ] ) ;
          CheckHasRaramBindingOrThrow( tmp2, argList[ 1 ] ) ;
          
          if ( tmp1 -> type == SPECIAL && tmp2 -> type == SPECIAL ) {
            if ( tmp1 -> lex == tmp2 -> lex ) {
              isInSameMemory = true ;
            } // if()
          } // if()
        } // else()
        
        if ( isInSameMemory ) {
          ansNode -> lex = "#t" ;
          
          argList.clear() ;
          return ansNode ;
        } // if()
        else {
          ansNode -> lex = "nil" ;
          
          argList.clear() ;
          return ansNode ;
        } // else()
      } // if()
      else if ( funcName == "equal?" ) { // compare the context
        // string originArgStr1 = argList[ 0 ] -> lex ;
        // string originArgStr2 = argList[ 1 ] -> lex ;
        Node* tmp1 = NULL ;
        Node* tmp2 = NULL ;
        tmp1 = EvaluateSExp( argList[ 0 ], level + 1 ) ;
        tmp2 = EvaluateSExp( argList[ 1 ], level + 1 ) ;
        CheckHasRaramBindingOrThrow( tmp1, argList[ 0 ] ) ;
        CheckHasRaramBindingOrThrow( tmp2, argList[ 1 ] ) ;
        argList[ 0 ] = tmp1 ;
        argList[ 1 ] = tmp2 ;
        
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
    Node* result = NULL ;
    
    if ( CountArgument( inTree ) == 2 || CountArgument( inTree ) == 3 ) {
      vector<Node*> argList ;
      GetArgumentList( inTree, argList ) ;
      
      // the first arguments should be the condition
      // if the evaluate of argment 1 is NULL then the format is wrong
      Node* condition = NULL ;
      condition = EvaluateSExp( argList[ 0 ], level + 1 ) ;
      if ( condition != NULL ) {
        if ( CountArgument( inTree ) == 2 ) {
          if ( condition -> lex != "#f" && condition -> lex != "nil" ) {
            result = EvaluateSExp( argList[ 1 ], level + 1 ) ;
          } // if()
          else ;
        } // if()
        else if ( CountArgument( inTree ) == 3 ) {
          if ( condition -> lex != "#f" && condition -> lex != "nil" ) {
            result = EvaluateSExp( argList[ 1 ], level + 1 ) ;
          } // if()
          else {
            result = EvaluateSExp( argList[ 2 ], level + 1 ) ;
          } // else()
        } // else if()
      } // if()
      else {
        gErrNode = argList[ 0 ] ;
        // throw new IncorrectArgumentTypeException( "if" ) ;
        throw new TestCondNotBoundException() ;
      } // else()
      
      argList.clear() ;
    } // if()
    else {
      throw new IncorrectNumberArgumentException( "if" ) ; // curious here
    } // else()
    
    return result ;
  } // ProcessIf()
  
  Node* ProcessCond( Node* inTree, int level ) {
    // cond expression cam take more than 1 arguments, at least one
    
    if ( CountArgument( inTree ) >= 1 ) {
      vector<Node*> argList ;
      GetArgumentList( inTree, argList ) ;
      
      // check each arguments should all be cons
      for ( int i = 0 ; i < ( int ) argList.size() ; i ++ ) {
        if ( argList[ i ] -> type != CONS
             || ! IsList( argList[ i ] )
             || CountArgument( argList[ i ] ) == 0 ) {
          gErrNode = inTree ;
          throw new CondFormatException() ;
        } // if()
      } // for()
      
      // start finding the first satisfying statement
      for ( int i = 0 ; i < ( int ) argList.size() ; i ++ ) {
        Node* condResult = NULL ;
        Node* condPart = NULL ;
        Node* statePart = NULL ;
        vector<Node*> subAugList ;
        GetArgumentList( argList[ i ], subAugList ) ;
        
        condPart = argList[ i ] -> left ;
        
        if ( i < ( int ) argList.size() - 1 ) {
          condResult = EvaluateSExp( condPart, level + 1 ) ;
          if ( condResult != NULL ) {
            if ( condResult -> lex != "#f"
                 && condResult -> lex != "nil"  ) {
              // assert: has deal with all the additional statement
              for ( int subI = 0 ; subI < ( int ) subAugList.size() ; subI ++ ) {
                if ( subI == ( int ) subAugList.size() - 1 ) {
                  statePart = subAugList[ subI ] ;
                } // if()
                else {
                  EvaluateSExp( subAugList[ subI ], level + 1 ) ;
                } // else()
              } // for()
              
              return EvaluateSExp( statePart, level + 1 ) ;
            } // if()
          } // if()
          else {
            gErrNode = condPart ;
            throw new TestCondNotBoundException() ;
          } // else()
        } // if()
        else {
          // the last condition can start with the key word "else"
          if ( condPart -> lex == "else" ) { // don't need to evaluate
            // assert: has deal with all the additional statement
            for ( int subI = 0 ; subI < ( int ) subAugList.size() ; subI ++ ) {
              if ( subI == ( int ) subAugList.size() - 1 ) {
                statePart = subAugList[ subI ] ;
              } // if()
              else {
                EvaluateSExp( subAugList[ subI ], level + 1 ) ;
              } // else()
            } // for()
            
            return EvaluateSExp( statePart, level + 1 );
          } // if()
          else {
            condResult = EvaluateSExp( condPart, level + 1 ) ;
            if ( condResult != NULL ) {
              if ( condResult -> lex != "#f"
                   && condResult -> lex != "nil" ) {
                // assert: has deal with all the additional statement
                for ( int subI = 0 ; subI < ( int ) subAugList.size() ; subI ++ ) {
                  if ( subI == ( int ) subAugList.size() - 1 ) {
                    statePart = subAugList[ subI ] ;
                  } // if()
                  else {
                    EvaluateSExp( subAugList[ subI ], level + 1 ) ;
                  } // else()
                } // for()
                
                if ( statePart != NULL && statePart -> type != EMPTY ) {
                  return EvaluateSExp( statePart, level + 1 ) ;
                } // if()
                else {
                  gErrNode = inTree ;
                  throw new CondFormatException() ;
                } // else()
              } // if()
              else ;
            } // if()
            else {
              gErrNode = condPart ;
              throw new TestCondNotBoundException() ;
            } // else()
          } // else()
        } // else()
        
        subAugList.clear() ;
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
      vector<Node*> argList ;
      GetArgumentList( inTree, argList ) ;
      
      Node* result = NULL ;
      
      for ( int i = 0 ; i < ( int ) argList.size() ; i ++ ) {
        if ( !IsList( argList[ i ] ) && argList[ i ] -> type == CONS ) {
          gErrNode = argList[ i ] ;
          throw new IncorrectArgumentTypeException( "begin" ) ;
        } // if()
      } // for()
      
      for ( int i = 0 ; i < ( int ) argList.size() ; i ++ ) {
        if ( i == ( int ) argList.size() - 1 ) {
          result = EvaluateSExp( argList[ i ], level + 1 ) ;
        } // if()
        else {
          EvaluateSExp( argList[ i ], level + 1 ) ;
        } // else()
      } // for()
      
      argList.clear() ;
      
      return result ;
    } // if()
    else {
      throw new IncorrectNumberArgumentException( "begin" ) ;
    } // else()
    
    return NULL ;
  } // ProcessBegin()
  
  Node* ProcessVerbose( string funcName, Node* inTree, int level ) {
    if ( funcName == "verbose" ) {
      if ( CountArgument( inTree ) == 1 ) {
        Node* arg = NULL ;
        arg = EvaluateSExp( inTree -> right -> left, level + 1 ) ;
        CheckHasRaramBindingOrThrow( arg, inTree -> right -> left ) ;
        if ( arg -> type == SPECIAL && arg -> lex == "nil" ) {
          gVerbose = false ;
          return gG.GetNullNode() ;
        } // if()
        else {
          gVerbose = true ;
          Node* node = new Node() ;
          node -> lex = "" ;
          node -> type = EMPTY ;
          node -> left = NULL ;
          node -> right = NULL ;
          node -> parent = NULL ;
          node -> isAddByMe = false ;
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
          Node* node = new Node() ;
          node -> lex = "" ;
          node -> type = EMPTY ;
          node -> left = NULL ;
          node -> right = NULL ;
          node -> parent = NULL ;
          node -> isAddByMe = false ;
          node -> lex = "#t" ;
          node -> type = SPECIAL ;
          return node ;
        } // if()
        else {
          return gG.GetNullNode() ;
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
      for ( Node* walk = localVars ; walk != NULL && walk -> lex != "nil" ; walk = walk -> right ) {
        if ( IsList( walk -> left ) && CountListElement( walk -> left ) == 2 ) {
          varNameList.push_back( walk -> left -> left -> lex ) ;
          varList.push_back( walk -> left ) ;
        } // if()
        else {
          varList.clear() ;
          return false ;
        } // else()
      } // for()
    } // if()
    else {
      varList.clear() ;
      return false ;
    } // else()
    
    if ( ( int ) varList.size() == 1 && varList[ 0 ] == NULL ) {
      varList.clear() ;
      return true ;
    } // if()
    else ;
    
    for ( int i = 0 ; i < ( int ) varList.size() ; i ++ ) {
      if ( varList[ i ] -> type == CONS ) { // Should be a cons structure
        
        string varName = "" ;
        Node* bind = NULL ;
        
        if ( varList[ i ] != NULL ) {
          if ( varList[ i ] -> left != NULL ) {
            varName = varList[ i ] -> left -> lex ;
          } // if()
          else ;
          
          if ( varList[ i ] -> right != NULL ) {
            bind = varList[ i ] -> right -> left ;
          } // if()
          else ;
        } // if()
        else {
          throw new Exception() ;
        } // else()
        
        if ( gG.IsSymbol( varName ) ) { // local variable should be a symbol
          
          Node* binding = NULL ;
          binding = EvaluateSExp( bind, level + 1 ) ;
          CheckHasReturnBindingOrThrow( binding, bind ) ;
          
          if ( binding == NULL ) {
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
    Node* allArg = NULL ;
    Node* localVarList = NULL ;
    Node* allSExp = NULL ;
    vector<string> localVarNameList ;
    
    allArg = inTree -> right ;
    localVarList = allArg -> left ;
    allSExp = allArg -> right ;
    
    if ( CountArgument( inTree ) >= 2 ) {
      // mCallStack.GetCleanLocalZone() ;
      if ( CheckAndStoreLocalVarSuccess( localVarList, localVarNameList, level + 1 ) ) {
        
        Node* walk = NULL ;
        for ( walk = allSExp ; walk -> right != NULL && walk -> right -> lex != "nil"
              ; walk = walk -> right ) {
          EvaluateSExp( walk -> left, level + 1 ) ;
        } // for()
        
        Node* finalResult = NULL ;
        finalResult = EvaluateSExp( walk -> left, level + 1 ) ; // the last expression result

        // mCallStack.RestoreLocalVar( localVarNameList ) ;
        //  mCallStack.ClearCurrentLocalVar() ;
        mCallStack.ClearRecentLocalVar( localVarNameList ) ;
        
        localVarNameList.clear() ;
        
        return finalResult ;
      } // if()
      else ;
    } // if()
    
    gErrNode = inTree ;
    throw new FormatException( "LET" ) ;
    return NULL ;
    
  } // ProcessLet()
  
  // use when the defining lambda
  int CountAndCkeckParameters( Node* arg, vector<string> &paraList, string funcName ) {
    int countNum = 0 ;
    paraList.clear() ;
    
    if ( IsList( arg ) ) { // the parameter part is a list
      for ( Node* walk = arg ; walk != NULL && walk -> lex != "nil" ; walk = walk -> right ) {
        if ( walk -> left != NULL && walk -> left -> type == ATOM ) {
          if ( gG.IsSymbol(  walk -> left -> lex ) ) {
            paraList.push_back( walk -> left -> lex ) ;
            countNum ++ ;
          } // if()
          else {
            gErrNode = arg ;
            throw new FormatException( funcName ) ;
          } // else()
        } // if()
        else {
          gErrNode = arg ;
          throw new FormatException( funcName ) ;
        } // else()
      } // for()
    } // if()
    else {
      gErrNode = arg ;
      throw new FormatException( funcName ) ;
    } // else()
    
    return countNum ;
  } // CountAndCkeckParameters()
  
  int CountAndCkeckParameters( Node* arg, vector<Node*> &paraList, int level ) {
    int countNum = 0 ;
    paraList.clear() ;
    
    for ( Node* walk = arg ; walk != NULL && walk -> lex != "nil" ; walk = walk -> right ) {
      paraList.push_back( walk -> left ) ;
      countNum ++ ;
    } // for()
    
    return countNum ;
  } // CountAndCkeckParameters()
  
  void ParameterBinding( vector<string> paramList, Node* bindings, string processName, int level ) {
    vector<Node*> bindingList ;
    vector<Node*> tmpBindingList ;
    CountAndCkeckParameters( bindings, bindingList, level + 1 ) ;
    
    if ( ( int ) paramList.size() == ( int ) bindingList.size() ) {
      for ( int i = 0 ; i < ( int ) paramList.size() ; i ++ ) {
        // the argument should be evaluate first, before binding to the symbol
        Node* bindValue = NULL ;
        bindValue = EvaluateSExp( bindingList[ i ], level + 1 ) ;
        
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
    // mCallStack.GetCleanLocalZone() ;
    // When in this function, there might be two circumstaces
    // 1. Not yet be evaluated
    // 2. Is the returned #<procedure lambda
    Node* lambdaProc = NULL ;
    lambdaProc = FindGlobalSymbol( "lambda" ) -> tree ;
    // int reserveIndex = FindGlobalSymbol( "lambda" ) ;
    // lambdaProc = mSymbolTable[ reserveIndex ].tree ;
    
    if ( inTree != NULL && inTree -> left != NULL && IsUserDefinedFunc( inTree -> left -> lex ) ) {
      return ProcessUserDefinedFunc( inTree, FindUserDefinedFunc( inTree -> left -> lex ), level + 1 );
    } // if()
    else if ( inTree != NULL && inTree -> left != NULL && inTree -> left -> type == CONS ) {
      // the second circumstances
      // mCallStack.GetCleanLocalZone() ;
      Node* finalResult = NULL ;
      
      if ( inTree -> right -> lex != "nil" ) { // immediately call the lambda function
        mLambdaStack.push_back( mLambdaFunc ) ;
        
        ParameterBinding( mLambdaFunc -> argList, inTree -> right, "lambda", level + 1 ) ;
        
        mLambdaFunc = mLambdaStack.back() ;
        mLambdaStack.pop_back() ;
      } // if()
      
      if ( mLambdaFunc -> tree != NULL ) {
        for ( Node* walk = mLambdaFunc -> tree ; walk != NULL && walk -> lex != "nil"
              ; walk = walk -> right ) {
          if ( walk -> right -> lex == "nil" ) {
            finalResult = EvaluateSExp( walk -> left, level + 1 ) ;
          } // if()
          else {
            EvaluateSExp( walk -> left, level + 1 ) ;
          } // else()
        } // for()
      } // if()
      
      // mCallStack.RestoreLocalVar( mLambdaFunc.argList ) ;
      // mCallStack.RestoreLocalVar( mLambdaVars ) ;
      // mCallStack.ClearCurrentLocalVar() ;
      mCallStack.ClearRecentLocalVar( mLambdaFunc -> argList ) ;
      
      return finalResult ;
    } // else if()
    else {
      // the left node of the input tree is "lambda", means this is the procedure of defineing
      // a lambda function
      
      if ( CountArgument( inTree ) >= 2 ) {
        Node* allArg = NULL ;
        Node* localVarList = NULL ;
        Node* allSExp = NULL ;
        
        allArg = inTree -> right ;
        localVarList = allArg -> left ;
        allSExp = allArg -> right ;
        
        if ( localVarList != NULL
             && ( localVarList -> type == CONS
                  || ( localVarList -> type == SPECIAL && localVarList -> lex == "nil" ) ) ) {
          
          try {
            
            mLambdaFunc = new Function() ;
            mLambdaFunc -> name = "" ;
            mLambdaFunc -> argNum =
            CountAndCkeckParameters( localVarList, mLambdaFunc -> argList, "LAMBDA" ) ;
            mLambdaFunc -> tree = allSExp ;
            
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
    
    Function* func = NULL  ;
    
    Node* treeInSymbolTable = NULL ;
    Node* finalResult = NULL ;
    
    if ( 0 <= funcIndex && funcIndex < ( int ) mUserDefinedFunctionTable.size() ) {
      func = mUserDefinedFunctionTable.at( funcIndex ) ;
    } // if()
    else {
      throw new Exception() ;
    } // else()
    
    treeInSymbolTable = FindGlobalSymbol( func -> name ) -> tree ;
    
    if ( treeInSymbolTable != NULL && treeInSymbolTable -> type == ATOM
         && treeInSymbolTable -> lex == "#<procedure lambda>" ) {
      ParameterBinding( func -> argList, inTree -> right, "lambda", level + 1 ) ;
    } // if()
    else if ( treeInSymbolTable != NULL ) {
      ParameterBinding( func -> argList, inTree -> right, func -> name, level + 1 ) ;
    } // else if()
    else ;
    
    mCallStack.RemoveLocalVarFromCurrentZone( func -> argList ) ;
    mCallStack.GetCleanLocalZone() ;
    mCallStack.RestoreLocalVar( func -> argList ) ;
    
    for ( Node* walk = func -> tree ; walk != NULL && walk -> lex != "nil" ; walk = walk -> right ) {
      if ( walk -> right != NULL && walk -> right -> lex == "nil" ) {
        finalResult = EvaluateSExp( walk -> left, level + 1 ) ;
      } // if()
      else {
        EvaluateSExp( walk -> left, level + 1 ) ;
      } // else()
    } // for()
    
    // mCallStack.RestoreLocalVar( func.argList ) ;
    mCallStack.ClearCurrentLocalVar() ;
    
    return finalResult ;
  } // ProcessUserDefinedFunc()
  
  Node* CreateErrorObject( string str ) {
    Node* errObj = new Node() ;
    errObj -> lex = "" ;
    errObj -> type = ERROR ;
    errObj -> parent = NULL ;
    errObj -> left = NULL ;
    errObj -> right = NULL ;
    errObj -> isAddByMe = false ;
    
    errObj -> lex = str ;
    
    return errObj ;
  } // CreateErrorObject()
  
  Node* ProcessCreateErrorObject( Node* inTree, int level ) {
    Node* result = NULL ;
    
    if ( CountArgument( inTree ) == 1 ) {
      if ( inTree -> right != NULL && inTree -> right -> left != NULL ) {
        Node* strNode = EvaluateSExp( inTree -> right -> left, level + 1 ) ;
        if ( strNode -> type == ATOM && gG.IsStr( strNode -> lex ) ) {
          result = CreateErrorObject( strNode -> lex ) ;
        } // if()
        else {
          gErrNode = inTree -> right -> left ;
          throw new IncorrectArgumentTypeException( "create-error-object" ) ;
        } // else()
      } // if()
      else ;
    } // if()
    else {
      gErrNode = inTree ;
      throw new IncorrectNumberArgumentException( "create-error-object" ) ;
    } // else()
    
    return result ;
  } // ProcessCreateErrorObject()
  
  bool IsErrorObj( Node* node ) {
    if ( node == NULL ) {
      return false ;
    } // if()
    else ;
    
    if ( node -> type == ERROR ) {
      return true ;
    } // if()
    else if ( node -> type == ATOM && SymbolExist( node -> lex ) ) {
      Node* bind = FindSymbolBinding( node -> lex ) ;
      
      if ( bind -> type == ERROR ) {
        return true ;
      } // if()
      else ;
    } // else if()
    else ;
    
    return false ;
  } // IsErrorObj()
  
  Node* ProcessIsError( Node* inTree, int level ) {
    Node* result = new Node() ;
    result -> lex = "nil" ;
    result -> type = SPECIAL ;
    result -> parent = NULL ;
    result -> left = NULL ;
    result -> right = NULL ;
    result -> isAddByMe = false ;
    
    if ( CountArgument( inTree ) == 1 ) {
      if ( inTree -> right != NULL && inTree -> right -> left != NULL ) {
        Node* objNode = inTree -> right -> left ;
        Node* binding = EvaluateSExp( objNode, level + 1 ) ;
        if ( binding != NULL && binding -> type == ERROR ) {
          result -> lex = "#t" ;
        } // if()
        else {
          CheckHasRaramBindingOrThrow( binding, objNode ) ;
        } // else()
      } // if()
      else ;
    } // if()
    else {
      gErrNode = inTree ;
      throw new IncorrectNumberArgumentException( "create-error-object" ) ;
    } // else()
    
    return result ;
  } // ProcessIsError()
  
  Node* ProcessDisplayString( Node* inTree, int level ) {
    Node* result = new Node() ;
    result -> lex = "" ;
    result -> type = ATOM ;
    result -> parent = NULL ;
    result -> left = NULL ;
    result -> right = NULL ;
    result -> isAddByMe = false ;
    
    if ( CountArgument( inTree ) == 1 ) {
      Node* target = NULL ;
      Node* evalResult = NULL ;
      
      target = inTree -> right -> left ;
      evalResult = EvaluateSExp( target, level + 1 ) ;
      
      if ( IsErrorObj( target ) ) {
        // Node* binding = FindSymbolBinding( target -> lex ) ;
        // CheckSymbolHasBindindRoThrow( target -> lex, binding, target ) ;
        
        if ( gG.IsStr( evalResult -> lex ) ) {
          result -> lex = evalResult -> lex ;
          
          cout << gG.GetStrContent( evalResult -> lex ) ;
        } // if()
        else {
          gErrNode = target ;
          throw new IncorrectArgumentTypeException( "display-string" ) ;
        } // else()
      } // if()
      else if ( evalResult -> type == ATOM && gG.IsStr( evalResult -> lex )  ) {
        string str = "" ;
        str = evalResult -> lex ;
        
        result -> lex = str ;
        
        // cout << gG.GetStrContent( str ) ;
        gG.PrintStr( gG.GetStrContent( str ) ) ;
      } // else if()
      else {
        gErrNode = target ;
        throw new IncorrectArgumentTypeException( "display-string" ) ;
      } // else()
    } // if()
    else {
      gErrNode = inTree ;
      throw new IncorrectNumberArgumentException( "display-string" ) ;
    } // else()
    
    return result ;
  } // ProcessDisplayString()
  
  Node* ProcessRead( Node* inTree, int level ) {
    Node* result = NULL ;
    bool grammerCorrect = false ;
    
    try {
      gLA.PeekToken() ;
      Token token = gLA.GetToken() ;
      
      grammerCorrect = gSA.CheckSExp( token ) ;
      
      if ( grammerCorrect ) {
        Tree tree ;
        tree.BuildTree() ;
        gG.Reset() ; // reset all information that used in Lexical analyzer
        
        result = tree.GetRoot() ;
      } // if()
      
    } // catch()
    catch ( NoClosingQuoteException* e ) {
      result = CreateErrorObject( "ERROR : END-OF-FILE encountered when there should be more input" ) ;
    } // catch()
    catch ( HighestException* e ) {
      result = CreateErrorObject( e -> Err_mesg() ) ;
    } // catch()
    
    return result ;
  } // ProcessRead()
  
  Node* ProcessWrite( Node* inTree, int level ) {
    Node* result = NULL ;
    
    if ( CountArgument( inTree ) == 1 ) {
      Node* target = inTree -> right -> left ;
      
      if ( target != NULL ) {
        Node* evalResult = NULL ;
        evalResult = EvaluateSExp( target, level + 1 ) ;
        CheckHasRaramBindingOrThrow( evalResult, target ) ;
        
        result = evalResult ;
        
        gG.PrettyPrint( evalResult ) ;
      } // if()
      else ;
    } // if()
    else {
      gErrNode = inTree ;
      throw new IncorrectNumberArgumentException( "write" ) ;
    } // else()
    
    return result ;
  } // ProcessWrite()
  
  Node* ProcessNewLine( Node* inTree, int level ) {
    Node* result = new Node() ;
    result -> lex = "nil" ;
    result -> type = SPECIAL ;
    result -> parent = NULL ;
    result -> left = NULL ;
    result -> right = NULL ;
    result -> isAddByMe = false ;
    
    if ( CountArgument( inTree ) == 0 ) {
      cout << endl ;
    } // if()
    else {
      gErrNode = inTree ;
      throw new IncorrectNumberArgumentException( "newline" ) ;
    } // else()
    
    return result ;
  } // ProcessNewLine()
  
  Node* ProcessSymbolToStr( Node* inTree, int level ) {
    Node* result = NULL ;
    
    if ( CountArgument( inTree ) == 1 ) {
      Node* target = NULL ;
      Node* evalResult = NULL ;
      target = inTree -> right -> left ;
      evalResult = EvaluateSExp( target, level + 1 ) ;
      CheckHasRaramBindingOrThrow( evalResult, target ) ;
      
      if ( evalResult != NULL && evalResult -> type == ATOM ) {
        result = new Node() ;
        result -> lex = "\"" + evalResult -> lex + "\"" ;
        result -> type = ATOM ;
        result -> parent = NULL ;
        result -> left = NULL ;
        result -> right = NULL ;
        result -> isAddByMe = false ;
      } // if()
      else {
        gErrNode = target ;
        throw new IncorrectArgumentTypeException( "symbol->string" ) ;
      } // else()
    } // if()
    else {
      gErrNode = inTree ;
      cout << "shit" << endl ;
      throw new IncorrectNumberArgumentException( "symbol->string" ) ;
    } // else()
    
    return result ;
  } // ProcessSymbolToStr()
  
  Node* ProcessNumberToStr( Node* inTree, int level ) {
    Node* result = NULL ;
    
    if ( CountArgument( inTree ) == 1 ) {
      Node* target = NULL ;
      Node* evalResult = NULL ;
      target = inTree -> right -> left ;
      evalResult = EvaluateSExp( target, level + 1 ) ;
      CheckHasRaramBindingOrThrow( evalResult, target ) ;
      
      if ( evalResult != NULL && evalResult -> type == ATOM ) {
        result = new Node() ;
        result -> lex = "" ;
        result -> type = ATOM ;
        result -> parent = NULL ;
        result -> left = NULL ;
        result -> right = NULL ;
        result -> isAddByMe = false ;
        
        if ( gG.IsINT( evalResult -> lex ) ) {
          result -> lex = "\"" + evalResult -> lex + "\"" ;
        } // if()
        else if ( gG.IsFLOAT( evalResult -> lex ) ) {
          result -> lex = "\"" + gG.FloatToStr( gG.GetValueOfFloatStr( evalResult -> lex ) ) + "\"" ;
        } // else if()
        else ;
      } // if()
      else {
        gErrNode = target ;
        throw new IncorrectArgumentTypeException( "symbol->string" ) ;
      } // else()
    } // if()
    else {
      gErrNode = inTree ;
      throw new IncorrectNumberArgumentException( "symbol->string" ) ;
    } // else()
    
    return result ;
  } // ProcessNumberToStr()
  
  Node* ProcessEval( Node* inTree, int level ) {
    Node* result = NULL ;
    
    if ( CountArgument( inTree ) == 1 ) {
      Node* target = NULL ;
      target = EvaluateSExp( inTree -> right -> left, level + 1 ) ;
      CheckHasRaramBindingOrThrow( target, inTree -> right -> left ) ;
      
      result = EvaluateSExp( target, 0 ) ;
      
    } // if()
    else {
      gErrNode = inTree ;
      throw new IncorrectNumberArgumentException( "eval" ) ;
    } // else ()
    
    return result ;
  } // ProcessEval()
  
  Node* ProcessSet( Node* inTree, int level ) {
    Node* result = NULL ;
    
    if ( CountArgument( inTree ) == 2 ) {
      string symName = "" ;
      symName = inTree -> right -> left -> lex ;
      
      Define( inTree, level + 1, true ) ;
      
      result = FindSymbolBinding( symName ) ;
    } // if()
    else {
      gErrNode = inTree ;
      throw new IncorrectNumberArgumentException( "set!" ) ;
    } // else()
    
    return result ;
  } // ProcessSet()
  
  string GetFuncNameFromFuncValue( string str ) {
    string name = "" ;
    
    if ( str == "" ) {
      return "" ;
    } // if()
    else {
      if ( str[ 0 ] == '#' ) {
        int whiteIndex = ( int ) str.find( ' ', 0 ) ;
        for ( int i = whiteIndex + 1 ; i < ( int ) str.length() ; i ++ ) {
          if ( i != ( int ) str.length() - 1 ) {
            name += str[ i ] ;
          } // if()
        } // for()
      } // if()
      else {
        name = str ;
      } // else()
    } // else()
    
    return name ;
  } // GetFuncNameFromFuncValue()
  
  void InitialLambdaFunc() {
    mLambdaFunc = new Function() ;
    mLambdaFunc -> name = "" ;
    mLambdaFunc -> argNum = 0 ;
    mLambdaFunc -> tree = NULL ;
    // mLambdaVars.clear() ;
  } // InitialLambdaFunc()
  
public:
  Evaluator() {
    AddOriginReserveWords() ;
    InitialReserveWord() ;
    InitialLambdaFunc() ;
  } // Evaluator()
  
  bool CheckSymbolHasBindindRoThrow( string symName, Node* result, Node* origin ) {
    if ( result == NULL ) {
      gErrNode = origin ;
      throw new UnboundValueException( symName ) ;
    } // if()
    else ;
    
    return true ;
  } // CheckSymbolHasBindindRoThrow()
  
  bool CheckHasReturnBinding( Node* result, Node* origin ) {
    if ( result == NULL ) {
      gErrNode = origin ;
      return false ;
    } // if()
    else ;
    
    return true ;
  } // CheckHasReturnBinding()
  
  bool CheckHasRaramBindingOrThrow( Node* result, Node* origin ) {
    if ( result == NULL ) {
      gErrNode = origin ;
      throw new ParamNotBoundException() ;
    } // if()
    else ;
    
    return true ;
  } // CheckHasRaramBindingOrThrow()
  
  bool CheckHasReturnBindingOrThrow( Node* result, Node* origin ) {
    if ( result == NULL ) {
      gErrNode = origin ;
      throw new NoReturnValueException() ;
    } // if()
    else ;
    
    return true ;
  } // CheckHasReturnBindingOrThrow()
  
  bool CheckTopLevelHasReturnBinding( Node* result, Node* origin ) {
    if ( result == NULL ) {
      gErrNode = origin ;
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
    
    try {
      // Two conditions in evaluating S-exp
      // 1. Atom
      // 2. CONS
      if ( treeRoot -> type == ATOM ) { // This S-exp is an Atom
        // may be p pure number or a symbol
        string atomStr = "" ;
        atomStr = treeRoot -> lex ;
        // this S-exp is a number
        if ( gG.IsINT( atomStr ) || gG.IsFLOAT( atomStr ) || gG.IsStr( atomStr ) ) {
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
      else if ( treeRoot -> type == ERROR ) {
        if ( SymbolExist( treeRoot -> lex ) ) {
          result = FindSymbolBinding( treeRoot -> lex ) ;
        } // if()
        else {
          CheckSymbolHasBindindRoThrow( treeRoot -> lex, result, treeRoot ) ;
        } // else()
        
        return treeRoot ;
      } // else if()
      else if ( treeRoot -> type == CONS ) { // This S-exp is a CONS
        Node* funcPart = NULL ;
        funcPart = treeRoot -> left ; // the left node of the root must be function part
        
        if ( funcPart -> type == ATOM ) { // if this is an atom, then check what it represents
          if ( SymbolExist( funcPart -> lex ) ) {
            Node* funcBinding = FindSymbolBinding( funcPart -> lex ) ;
            if ( funcBinding == NULL ) {
              throw new UnboundValueException( funcPart -> lex ) ;
            } // if()
            else ;
            
            originFuncName = funcBinding -> lex ;
          } // if()
          else {
            originFuncName = funcPart -> lex ;
          } // else()
        } // if()
        else if ( funcPart -> type == CONS ) {
          Node* funcResult = NULL ;
          funcResult = EvaluateSExp( funcPart, level + 1 ) ;
          
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
      if ( originFuncName != "" && ( originFuncName[ 0 ] == '#' || originFuncName == "lambda" ) ) {
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
            result = EvaluateCONS( treeRoot, level + 1 ) ;
          } // else if()
          else if ( funcName == "list" ) {
            result = EvaluateLIST( treeRoot, level + 1 ) ;
          } // else if()
          else if ( funcName == "define" ) {
            result = Define( treeRoot, level + 1, false ) ;
          } // else if()
          else if ( funcName == "car" || funcName == "cdr" ) {
            result = AccessList( funcName, treeRoot, level + 1 ) ;
          } // else if()
          else if ( IsPredicator( funcName ) ) {
            result = PrimitivePredecates( funcName, treeRoot, level + 1 ) ;
          } // else if()
          else if ( IsBasicOperation( funcName ) ) {
            result = ProcessOperation( funcName, treeRoot, level + 1 );
          } // else if()
          else if ( funcName == "eqv?" || funcName == "equal?" ) {
            result = ProcessEqvAndEqual( funcName, treeRoot, level + 1 );
          } // else if()
          else if ( funcName == "if" ) {
            result = ProcessIf( treeRoot, level + 1 ) ;
          } // else if()
          else if ( funcName == "cond" ) {
            result = ProcessCond( treeRoot, level + 1 ) ;
          } // else if()
          else if ( funcName == "begin" ) {
            result = ProcessBegin( treeRoot, level + 1 ) ;
          } // else if()
          else if ( funcName == "let" ) {
            result = ProcessLet( treeRoot, level + 1 ) ;
          } // else if()
          else if ( funcName == "lambda" ) {
            result = ProcessLambda( treeRoot, level + 1 ) ;
          } // else if()
          else if ( funcName == "verbose" || funcName == "verbose?" ) {
            result = ProcessVerbose( funcName, treeRoot, level + 1 ) ;
          } // else if()
          else if ( funcName == "create-error-object" ) {
            result = ProcessCreateErrorObject( treeRoot, level + 1 ) ;
          } // else if()
          else if ( funcName == "error-object?" ) {
            result = ProcessIsError( treeRoot, level + 1 ) ;
          } // else if()
          else if ( funcName == "display-string" ) {
            result = ProcessDisplayString( treeRoot, level + 1 ) ;
          } // else if()
          else if ( funcName == "read" ) {
            result = ProcessRead( treeRoot, level + 1 ) ;
          } // else if()
          else if ( funcName == "write" ) {
            result = ProcessWrite( treeRoot, level + 1 ) ;
          } // else if()
          else if ( funcName == "newline" ) {
            result = ProcessNewLine( treeRoot, level + 1 ) ;
          } // else if()
          else if ( funcName == "symbol->string" ) {
            result = ProcessSymbolToStr( treeRoot, level + 1 ) ;
          } // else if()
          else if ( funcName == "number->string" ) {
            result = ProcessNumberToStr( treeRoot, level + 1 ) ;
          } // else if()
          else if ( funcName == "eval" ) {
            result = ProcessEval( treeRoot, level + 1 ) ;
          } // else if()
          else if ( funcName == "set!" ) {
            result = ProcessSet( treeRoot, level + 1 ) ;
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
          else ;
        } // if()
        else if ( definedFuncIndex != -1 ) { // this user-defined function exist
          // process the user defined function
          result = ProcessUserDefinedFunc( treeRoot, definedFuncIndex, level + 1 ) ;
        } // else if()
        else { // either a function name or a symbol
          gErrNode = EvaluateSExp( treeRoot -> left, level + 1 ) ;
          throw new ApplyNonFunctionException() ; // curious
        } // else()
      } // if()
      else {
        gErrNode = treeRoot ;
        throw new NonListException() ; // curious
      } // else()
    } catch ( Exception* e ) {
      gErrNode = treeRoot -> left ;
      throw e ;
    } // catch()
    
    return result ;
  } // EvaluateSExp()
  
  void Clean() {
    mUserDefinedFunctionTable.clear() ;
    CleanWholeStack() ;
    
    ResetSymbolTable() ;
    ResetReserveWord() ;
  } // Clean()
  
  void CleanLocalVariables() {
    mCallStack.ClearCurrentLocalVar() ;
    InitialLambdaFunc() ;
  } // CleanLocalVariables()
  
  void CleanWholeStack() {
    CleanLocalVariables() ;
    mCallStack.CleanStack() ;
  } // CleanWholeStack()
  
} ; // Evaluator

bool IsDefineOrCleanSExp( Node* node ) {
  if ( node != NULL && node -> type == CONS && node -> left != NULL ) {
    if ( node -> left -> lex == "define" || node -> left -> lex == "clean-environment" ) {
      return true ;
    } // if()
    else ;
  } // if()
  else ;
  
  return false ;
} // IsDefineOrCleanSExp()

Evaluator gEval ;

int main() {
  
  try {
    
    bool grammerCorrect = false ;
    
    cin >> uTestNum ;
    char retuenLine = '\0' ;
    retuenLine = cin.get() ;
    
    cout << "Welcome to OurScheme!" << endl ;
    string inputStr = "" ;
    
    while ( !gIsEOF ) {
      
      try {
        
        cout << endl << "> " ;
        
        gLA.PeekToken() ;
        Token token = gLA.GetToken() ;
        
        grammerCorrect = gSA.CheckSExp( token ) ;
        if ( grammerCorrect ) {
          uTree.BuildTree() ;
          gG.Reset() ; // reset all information that used in Lexical analyzer
          
          if ( !gIsEOF ) {
            // gG.PrettyPrint( uTree.GetRoot() ) ; // proj.1
            
            try {
              // Evaluate the tree and start with level 0
              Node* result = NULL ;
              result = gEval.EvaluateSExp( uTree.GetRoot(), 0 ) ;
              // check whether the result has a binding
              // Two exceptions: 1.define 2. clean-environment
              if ( !gEval.CheckTopLevelHasReturnBinding( result, uTree.GetRoot() )
                   && !IsDefineOrCleanSExp( uTree.GetRoot() ) ) {
                gErrNode = uTree.GetRoot() ;
                throw new NoReturnValueException() ;
              } // if()
              else if ( result != NULL ) {
                gG.PrettyPrint( result ) ;
                cout << endl ;
              } // else if()
              else ;
              
            } // try
            catch ( LevelException* e ) {
              cout << e -> Err_mesg() << endl ;
            } // catch()
            catch ( NonListException* e ) {
              cout << e -> Err_mesg() ;
              gG.PrettyPrint( gErrNode ) ;
              cout << endl ;
            } // catch()
            catch ( UnboundValueException* e ) {
              cout << e -> Err_mesg() << endl ;
            } // catch()
            catch ( ApplyNonFunctionException* e ) {
              cout << e -> Err_mesg() ;
              gG.PrettyPrint( gErrNode ) ;
              cout << endl ;
            } // catch()
            catch ( IncorrectNumberArgumentException* e ) {
              cout << e -> Err_mesg() << endl ;
            } // catch()
            catch ( IncorrectArgumentTypeException* e ) {
              cout << e -> Err_mesg() ;
              gG.PrettyPrint( gErrNode ) ;
              cout << endl ;
            } // catch()
            catch ( NoReturnValueException* e ) {
              cout << e -> Err_mesg() ;
              gG.PrettyPrint( gErrNode ) ;
              cout << endl ;
              // cout << endl ;
            } // catch()
            catch ( ParamNotBoundException* e ) {
              cout << e -> Err_mesg() ;
              gG.PrettyPrint( gErrNode ) ;
              cout << endl ;
            } // catch()
            catch ( DivideByZeroException* e ) {
              cout << e -> Err_mesg() << endl ;
            } // catch()
            catch ( DefineFormatException* e ) {
              // cout << e -> Err_mesg() << endl ;
              cout << e -> Err_mesg() ;
              gG.PrettyPrint( gErrNode ) ;
              cout << endl ;
            } // catch()
            catch ( TestCondNotBoundException* e ) {
              cout << e -> Err_mesg() ;
              gG.PrettyPrint( gErrNode ) ;
              cout << endl ;
            } // catch()
            catch ( CondNotBoundException* e ) {
              cout << e -> Err_mesg() ;
              gG.PrettyPrint( gErrNode ) ;
              cout << endl ;
            } // catch()
            catch ( CondFormatException* e ) {
              cout << e -> Err_mesg() ;
              gG.PrettyPrint( gErrNode ) ;
              cout << endl ;
              // cout << endl ;
            } // catch()
            catch ( FormatException* e ) {
              cout << e -> Err_mesg() ;
              gG.PrettyPrint( gErrNode ) ;
              cout << endl ;
            } // catch()
            catch ( NonReturnAssignedException* e ) {
              cout << e -> Err_mesg() ;
              gG.PrettyPrint( gErrNode ) ;
              cout << endl ;
            } // catch()
            catch ( Exception* e ) {
              cout << e -> Err_mesg() ;
              gG.PrettyPrint( gErrNode ) ;
              cout << endl ;
            } // catch()
          } // if()
          
          // uTree.DeleteTree() ;
        } // if()
        
      } // catch()
      catch ( EOFException* e ) {
        cout << e -> Err_mesg() << endl ;
        gJustFinishAExp = true ;
        cout << "Thanks for using OurScheme!" ;
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
      
      gJustFinishAExp = true ;
      gEval.CleanWholeStack() ;
      gG.Reset() ;
    } // while()
    
    gEval.CleanWholeStack() ;
    gG.Reset() ;
    gEval.Clean() ;
    
    cout << endl << "Thanks for using OurScheme!" ;
    
  } catch ( exception* e ) {
    cout << "exception: " << e -> what() << "\n" ;
  } // catch()
  
  return 0 ;
  
} // main()
