
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

struct Node {
  string lex ; // the string (what it looks in the input file) of this token
  NodeType type ; // Three possibility: 1.Atom  2.Special(NIL)  3.Cons
  Node* left ;
  Node* right ;
  Node* parent ;
  
  // Node() : lex(""), type(EMPTY), left(NULL), right(NULL), parent(NULL) {} ;
};

static int uTestNum = 0 ;  // test num from PAL
int gLine = 1 ;  // the line of the token we recently "GET"
int gColumn = 0 ; // // the golumn of the token we recently "GET"
string gPeekToken = "" ;  // the recent token we peek BUT haven't "GET"
bool gIsEOF = false ; // if is TRUE means there doesn't have '(exit)'
bool gJustFinishAExp = false ;

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
    
    mRoot = NULL ;
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
  
  string IntToStr( int num ) {
    string str = "" ;
    bool isNegative = false ;
    if ( num == 0 ) return "0" ;
    else if ( num < 0 ) {
      isNegative = true ;
      num *= -1 ; // change it to positive for calculating
    } // else()
    
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
  
  Node* GetNullNode() {
    Node* nullNode = new Node ;
    nullNode -> lex = "nil" ;
    nullNode -> type = SPECIAL ;
    nullNode -> parent = NULL ;
    nullNode -> left = NULL ;
    nullNode -> right = NULL ;
    
    return nullNode ;
  } // GetNullNode()
  
  Node* GetEmptyNode() {
    Node* nullNode = new Node ;
    nullNode -> lex = "" ;
    nullNode -> type = EMPTY ;
    nullNode -> parent = NULL ;
    nullNode -> left = NULL ;
    nullNode -> right = NULL ;
    
    return nullNode ;
  } // GetEmptyNode()
  
  void Reset() {
    gLine = 1 ;
    gColumn = 0 ;
    gOriginalList.Clear() ;
    gPeekToken = "" ;
  } // Reset()
  
  void SkipLine() {
    char ch = cin.peek() ;
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
    + " Column " + g.IntToStr( mCol - ( int ) mStr.length() + 1 ) + " is >>" + mStr + "<<" ;
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
    while ( keepRead && !IsReturnLine( ch_peek ) && !g.IsEOF( ch_peek ) ) {
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
    if ( lex != "." && g.IsFLOAT( lex ) ) {
      // assert: float with a original format
      // now  can start trandfer the float into the format which (int).(3 chars)
      token.str = FormatFloat( lex ) ;
    } // if()
    else {
      token.str = lex ;
    } // else()
    
    token.line = gLine ;
    token.column = gColumn - ( int ) lex.length() + 1 ;
    token.type = g.GetTokenType( lex ) ;
    
    return token ;
  } // LexToToken()
  
public:
  
  // Purpose: accept the token string from func. GetToken(), and response the corresponding token value
  
  string PeekToken() {
    string tokenStrWeGet = "" ;
    
    if ( gPeekToken == "" ) {
      char ch = '\0' ;
      
      // peek whether the next char is in input
      ch = cin.peek() ;
      if ( g.IsEOF( ch ) ) { // -1 means -1 for cin.peek
        gIsEOF = true ;
        throw EOFException() ;
      } // if()
      
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
        
        ch = cin.peek() ;  // keep peeking next char
        if ( g.IsEOF( ch ) ) { // -1 means -1 for cin.peek
          gIsEOF = true ;
          throw EOFException() ;
        } // if()
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
        if ( g.IsEOF( ch ) ) { // -1 means -1 for cin.peek
          gIsEOF = true ;
          throw EOFException() ;
        } // if()
        
        // check whether EOF because we may encounter EOF while making a peek token
        while ( !IsSeparator( ch ) && !IsWhiteSpace( ch ) && ( int ) ch != -1 ) {
          ch = GetChar() ;
          tokenStrWeGet += ch ;
          ch = cin.peek() ;
          if ( g.IsEOF( ch ) ) { // -1 means -1 for cin.peek
            gIsEOF = true ;
            throw EOFException() ;
          } // if()
        } // while()
        
      } // if()
      else if ( ch == '\"' ) {
        // assert: we get the whole token
        tokenStrWeGet = GetFullStr( tokenStrWeGet ) ;
      } // else if()
      
      // assert: we get the whole token
      gPeekToken = tokenStrWeGet ;
    } // if()
    
    gJustFinishAExp = false ;
    
    return gPeekToken ;
    
  } // PeekToken()
  
  Token GetToken() {
    if ( gPeekToken == "" ) PeekToken() ;
    
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
              throw MissingAtomOrLeftParException( gLine, gColumn, gPeekToken ) ;
              return false ;
            } // if()
            else {
              if ( g.GetTokenType( mLa.PeekToken() ) == RPAREN ) {
                token = mLa.GetToken() ; // must be >)<
                
                return true ;
              } // if()
              else {
                throw MissingRightParException( gLine, gColumn, gPeekToken ) ;
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

// --------------------- Error Definition Proj.2 (start) ---------------------

class NonListException {
private:
  Node* mErrNode ;
public:
  NonListException( Node* errTree ) {
    mErrNode = errTree ;
  } // NonListException()
  
  string Err_mesg() {
    string mesg = "ERROR (non-list): " ;
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
  string mLex ;
  
public:
  IncorrectArgumentTypeException( string errFuncName, string errLex ) {
    mFuncName = errFuncName ;
    mLex = errLex ;
  } // IncorrectArgumentTypeException()
  
  string Err_mesg() {
    string mesg = "ERROR (" + mFuncName + " with incorrect argument type) : " + mLex ;
    return mesg ;
  } // Err_mesg()
} ; // IncorrectArgumentTypeException

class ApplyNonFunctionException {
private:
  string mLex ;
  Node* mTree ;
  
public:
  ApplyNonFunctionException( string errLex, Node* errTree ) {
    mLex = errLex ;
    mTree = errTree ;
  } // ApplyNonFunctionException()
  
  string Err_mesg() {
    string mesg = "ERROR (attempt to apply non-function) : " ;
    return mesg ;
  } // Err_mesg()
  
  void PrintErrAtom() {
    if ( mTree == NULL ) { // print the lex
      cout << mLex << endl ;
    } // if()
    else { // the error atom is a symbol, so need a transformation
      g.PrettyPrint( mTree ) ;
    } // else()
  } // PrintErrAtom()
} ; // ApplyNonFunctionException

class NoReturnValueException {
private:
  Node* mErrNode ;
public:
  NoReturnValueException( Node* err ) {
    mErrNode = err ;
  } // NoReturnValueException()
  
  string Err_mesg() {
    string mesg = "ERROR (no return value) : " ;
    return mesg ;
  } // Err_mesg()
  
  Node* Err_node() {
    return mErrNode ;
  } // Err_node()
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
private:
  Node* mErrNode ;
public:
  DefineFormatException( Node* err ) {
    mErrNode = err ;
  } // DefineFormatException()
  
  string Err_mesg() {
    string mesg = "ERROR (DEFINE format) : " ;
    return mesg ;
  } // Err_mesg()
  
  Node* Err_node() {
    return mErrNode ;
  } // Err_node()
} ; // DefineFormatException

class CondFormatException {
private:
  Node* mErrNode ;
public:
  CondFormatException( Node* err ) {
    mErrNode = err ;
  } // CondFormatException()
  
  string Err_mesg() {
    string mesg = "ERROR (COND format) : " ;
    return mesg ;
  } // Err_mesg()
  
  Node* Err_node() {
    return mErrNode ;
  } // Err_node()
} ; // CondFormatException

// --------------------- Error Definition Proj.2 (end) ---------------------

// Purpose: Do the evaluation and store the user definitions
class Evaluator {
private:
  string mOriginReserveWordList[ 47 ] = { "cons", "list", "quote", "define", "car", "cdr", "not", "and", "or", "begin", "if", "cond",  "clean-environment", "quote", "'", "atom?", "pair?", "list?", "null?", "integer?", "real?", "number?", "string?",  "boolean?", "symbol?", "+", "-", "*", "/", ">", ">=", "<", "<=", "=", "and", "not", "or", "string-append", "string>?", "string<?", "string=?", "eqv?", "equal?", "begin", "if", "cond", "exit" } ;
  
  struct Symbol {
    string name ;
    Node* tree ;
  } ; //SymbolInfo
  
  struct Function {
    string name ;
    int augNum ;
    Node* tree ;
  } ; // Function
  
  struct ReserveWord {
    string name ;
    vector<string> list ;
  } ; // ReserveWord
  
  vector<Symbol> mSymbolTable ;
  vector<Function> mFunctionTable ;
  vector<ReserveWord> mReserveWords ;
  
  void ResetReserveWord() {
    for ( int i = 0 ; i < 47 ; i ++ ) {
      ReserveWord tmpWord ;
      tmpWord.name = mOriginReserveWordList[ i ] ;
      tmpWord.list.clear() ;
      mReserveWords.push_back( tmpWord ) ;
    } // for()
  } // ResetReserveWord()
  
  void AddSymbol( string symName, Node* assignedTree ) {
    Symbol symbol ;
    symbol.name = "" ;
    symbol.tree = NULL ;
    
    symbol.name = symName ;
    symbol.tree = assignedTree ;
    
    mSymbolTable.push_back( symbol ) ; // add this new symbol to the table
  } // AddSymbol()
  
  void AddFunction( string funcName, int numberOfAug, Node* assignedTree ) {
    Function func ;
    func.name = "" ;
    func.augNum = 0 ;
    func.tree = NULL ;
    
    func.name = funcName ;
    func.augNum = numberOfAug ;
    func.tree = assignedTree ;
    
    mFunctionTable.push_back( func ) ;
  } // AddFunction()
  
  string GetReserveWordType( string str ) {
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
  
  bool IsPredicator( string str ) {
    if ( str == "atom?" || str == "pair?" || str == "list?" || str == "null?" || str == "integer?" || str == "real?" || str == "number?" || str == "string?" || str == "boolean?" || str == "symbol?" ) {
      return true ;
    } // if()
    
    return false ;
  } // IsPredicator()
  
  int FindDefinedSymbol( string str ) {
    int index = -1 ;
    
    for ( int i = 0 ; i < mSymbolTable.size() && index == -1 ; i ++ ) {
      if ( str == mSymbolTable[ i ].name ) {
        index = i ;
      } // if()
    } // for()
    
    return index ;
  } // FindDefinedSymbol()
  
  int FindDefinedFunc( string str ) {
    int index = -1 ;
    
    for ( int i = 0 ; i < mFunctionTable.size() && index == -1 ; i ++ ) {
      if ( str == mFunctionTable[ i ].name ) {
        index = i ;
      } // if()
    } // for()
    
    return index ;
  } // FindDefinedFunc()
  
  bool IsList( Node* originRoot, Node* root ) {
    if ( root -> type == ATOM || root -> type == SPECIAL ) { // the last node (should be an atom)
      if ( root -> lex == "nil" || root == originRoot ) {
        return true ;
      } // if()
      
      return false ;
    } // if()
    
    return IsList( originRoot, root -> right ) ;
  } // IsList()
  
  int CountArgument( Node* tree ) {
    if ( tree -> right == NULL ) {
      return -1 ;
    } // if()
    
    return CountArgument( tree -> right ) + 1 ;
  } // CountArgument()
  
  bool IsSymbol( string str ) {
    if ( g.GetTokenType( str ) == SYMBOL ) {
      return true ;
    } // if()
    
    return false ;
  } // IsSymbol()
  
  // Purpose combined the trees
  Node* EvaluateCONS( Node* inTree ) {
    Node* consNode = NULL ;
    // Step1. check argument num ( cons can only have 2 )
    if ( CountArgument( inTree ) == 2 ) {
      // Step2. check argument type
      // in CONS case, both arguments of CONS should be a tree (list)
      Node* firstArg = inTree -> right -> left ; // assume aug1 is a list
      Node* secondArg = inTree -> right -> right -> left ; // assume aug2 is a list
      
      if ( IsList( firstArg, firstArg ) ) {
        if ( firstArg -> type == ATOM
             && IsSymbol( firstArg -> lex  )
             && FindDefinedSymbol( firstArg -> lex ) == -1 ) {
          throw UnboundValueException( firstArg -> lex ) ;
        } // if()
        else {
          if ( IsList( secondArg, secondArg ) ) {
            if ( secondArg -> type == ATOM
                 && IsSymbol( secondArg -> lex )
                 && FindDefinedSymbol( secondArg -> lex ) == -1 ) {
              throw UnboundValueException( secondArg -> lex ) ;
            } // if()
            else {
              // Step3. If no error create a new Node
              consNode = new Node ;
              consNode -> lex = "" ;
              consNode -> type = CONS ;
              consNode -> left = NULL ;
              consNode -> right = NULL ;
              consNode -> parent = NULL ;
              
              consNode -> left = EvaluateSExp( firstArg ) ;
              consNode -> right = EvaluateSExp( secondArg ) ;
            } // else()
          } // if()
          else {
            throw NonListException( secondArg ) ;
          } // else()
        } // else()
      } // if()
      else {
        throw NonListException( firstArg ) ;
      } // else()
    } // if()
    else {
      throw IncorrectNumberArgumentException( "cons" ) ;
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
  
  Node* EvaluateLIST( Node* inTree ) {
    Node* result = NULL ;
    vector<Node*> argList ;
    if ( CountArgument( inTree ) > 0 ) {
      // Step1. find out all the arguments and put them in a list
      argList = GetArgumentList( inTree )  ;
      // Step2. start to check the type and mean time replace the symbol
      for ( int i = 0 ; i < argList.size() ; i ++ ) {
        if ( IsList( argList[ i ], argList[ i ] ) ) {
          if ( argList[ i ] -> type == ATOM
              && IsSymbol( argList[ i ] -> lex ) ) {
            int symIndex = FindDefinedSymbol( argList[ i ] -> lex ) ;
            if ( symIndex != -1 ) {
              argList[ i ] = EvaluateSExp( mSymbolTable[ symIndex ].tree ) ;
            } // if()
            else {
              throw UnboundValueException( argList[ i ] -> lex ) ;
            } // else()
          } // if()
          else { // this is a list, but need to check more detail
            argList[ i ] = EvaluateSExp( argList[ i ] ) ;
          } // else()
        } // if()
        else {
          throw NonListException( argList[ i ] ) ;
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
        
        node -> left = argList[ i ] ;
        
        if ( i == 0 ) {
          result = node ;
          prevNode = node ;
        } // if()
        else if ( i == argList.size() - 1 ){
          prevNode -> right = node ;
          node -> parent = prevNode ;
          
          Node* nilNode = new Node ;
          nilNode -> lex = "nil" ;
          nilNode -> type = ATOM ;
          nilNode -> parent = node ;
          nilNode -> left = NULL ;
          nilNode -> right = NULL ;
          node -> right = nilNode ;
          
        } // else if()
        else {
          prevNode -> right = node ;
          node -> parent = prevNode ;
          
          prevNode = node ;
        } // else()
      } // for()

      return result ;
    } // if()
    
    return NULL ; // it is empty in the argument
  } // EvaluateLIST()
  
  void AddNewReserveWord( string reserveName, string newName ) {
    for ( int i = 0 ; i < 47 ; i ++ ) {
      if ( reserveName == mReserveWords[ i ].name ) {
        mReserveWords[ i ].list.push_back( newName ) ;
        return ;
      } // if()
    } // for()
  } // AddNewReserveWord()
  
  Node* Define( Node* inTree ) {
    vector<Node*> argList = GetArgumentList( inTree ) ;
    
    if ( CountArgument( inTree ) == 2 ) {
      Symbol newSymbol ;
      newSymbol.name = "" ;
      newSymbol.tree = g.GetEmptyNode() ;
      // the first argument should be a symbol
      if ( argList[ 0 ] -> type == ATOM ) {
        string reserveName = GetReserveWordType( argList[ 0 ] -> lex ) ;
        if ( reserveName == "" ) {
          if ( g.GetTokenType( argList[ 0 ] -> lex ) == SYMBOL ) {
            
            int symIndex = FindDefinedSymbol( argList[ 0 ] -> lex ) ;
            // check the be binded s-exp is correct
            EvaluateSExp( argList[ 1 ] ) ;
            
            if ( symIndex != -1 ) { // this symbol has already exist, update it
              
              if ( FindDefinedSymbol( argList[ 1 ] -> lex ) == -1 ) {
                // the reference value is not a symbol
                newSymbol.name = argList[ 0 ] -> lex ;
                mSymbolTable[ symIndex ].tree = argList[ 1 ] ; // copy
              } // if()
              else {
                string reserveName = GetReserveWordType( argList[ 1 ] -> lex ) ;
                if ( reserveName != "" ) {
                  // this is a special case, define your own reserve word
                  // add this to the reserveWordList
                  AddNewReserveWord( reserveName, argList[ 0 ] -> lex ) ;
                } // if()
                
                mSymbolTable[ symIndex ].tree =
                mSymbolTable[ FindDefinedSymbol( argList[ 1 ] -> lex ) ].tree ;
              } // else()
            } // if()
            else {
              newSymbol.name = argList[ 0 ] -> lex ;
              if ( FindDefinedSymbol( argList[ 1 ] -> lex ) == -1 ) {
                // the reference value is not a symbol
                newSymbol.tree = argList[ 1 ] ; // copy
              } // if()
              else {
                string reserveName = GetReserveWordType( argList[ 1 ] -> lex ) ;
                if ( reserveName != "" ) {
                  // this is a special case, define your own reserve word
                  // add this to the reserveWordList
                  AddNewReserveWord( reserveName, argList[ 0 ] -> lex ) ;
                } // if()
                
                newSymbol.tree =
                mSymbolTable[ FindDefinedSymbol( argList[ 1 ] -> lex ) ].tree ;
              } // else()
              mSymbolTable.push_back( newSymbol ) ;
            } // else()
            
            cout << newSymbol.name << " defined" << endl ;
          } // if()
          else {
            throw IncorrectArgumentTypeException( "define", argList[ 0 ] -> lex ) ;
          } // else()
        } // if()
        else { // user try to re-define the reserve word
          throw DefineFormatException( inTree ) ;
        } // else()
      } // if()
      else { //
        throw IncorrectArgumentTypeException( "define", argList[ 0 ] -> left -> lex ) ;
      } // else()
    } // if()
    else {
      throw DefineFormatException( inTree ) ;
    } // else()
    
    return NULL ;
  } // Define()
  
  Node* AccessList( string funcName, Node* inTree ) {
    if ( CountArgument( inTree ) == 1 ){
      Node* targetTree = EvaluateSExp( inTree -> right -> left ) ;
      
      if ( targetTree != NULL ) {
        if ( targetTree -> type == ATOM || targetTree -> type == SPECIAL ) {
          if ( g.IsINT( targetTree -> lex ) || g.IsFLOAT( targetTree -> lex ) ) {
            throw IncorrectArgumentTypeException( funcName, targetTree -> lex ) ;
          } // if()
          else {
            return targetTree ;
          } // else()
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
        throw IncorrectArgumentTypeException( funcName, inTree -> right -> left -> lex ) ;
      } // else()
    } // if()
    else {
      throw IncorrectNumberArgumentException( funcName ) ;
    } // else()
    
    return NULL ;
  } // AccessList()
  
  Node* PrimitivePredecates( string func, Node* inTree ) {
    bool ans = false ;
    Node* ansNode = new Node ;
    ansNode -> lex="#t" ;
    ansNode -> type=SPECIAL ;
    ansNode -> parent=NULL ;
    ansNode -> left=NULL ;
    ansNode -> right=NULL ;
    // can only have ONE argement
    if ( CountArgument( inTree ) == 1 ) {
      Node* target = EvaluateSExp( inTree -> right -> left ) ;
      
      if ( target != NULL ) {
        if ( func == "atom?" ) {
          if ( target -> type == ATOM ) {
            ans = true ;
          } // if()
        } // if()
        else if ( func == "pair?" ) {
          if ( target -> type == CONS && CountArgument( target ) <= 1 ) {
            ans = true ;
          } // if()
        } // else if()
        else if ( func == "list?" ) {
          if ( target -> type == CONS && IsList( target, target ) ) {
            ans = true ;
          } // if()
        } // else if()
        else if ( func == "null?" ) {
          if ( target -> type == SPECIAL
              && ( target -> lex == "nil" || target -> lex == "#f" ) ) {
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
          if ( target -> type == ATOM && g.GetTokenType( func ) == SYMBOL ) {
            ans = true ;
          } // if()
        } // else if()
      } // if()
      else {
        throw IncorrectArgumentTypeException( func, inTree -> right -> left -> lex ) ;
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
      throw IncorrectNumberArgumentException( func ) ;
    } // else()
    
    return NULL ;
  } // PrimitivePredecates()
  
  Node* CleanEnvironment() {
    mSymbolTable.clear() ;
    mFunctionTable.clear() ;
    mReserveWords.clear() ;
    AddOriginReserveWords() ;
    ResetReserveWord() ;
    cout << "environment-cleaned" << endl ;
    
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
  
  Node* ProcessMath( string funcName, vector<Node*> argList ) {
    Node* ansNode = new Node ;
    ansNode -> lex = "" ;
    ansNode -> type = ATOM ;
    ansNode -> parent = NULL ;
    ansNode -> left = NULL ;
    ansNode -> right = NULL ;
    
    // check whether all the arguments are numbers
    for ( int i = 0 ; i < argList.size() ; i ++ ) {
      Node* currentAug = EvaluateSExp( argList[ i ] ) ;
      if ( currentAug != NULL && currentAug -> type == ATOM
          && ( g.IsINT( currentAug -> lex )
              || g.IsFLOAT( currentAug -> lex ) ) ) {
        argList[ i ] = EvaluateSExp( argList[ i ] ) ;
      } // if()
      else { // a non number atom exist
        throw IncorrectArgumentTypeException( funcName, currentAug -> lex ) ;
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
            throw DivideByZeroException() ;
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
            throw DivideByZeroException() ;
          } // else()
        } // else if()
      } // else if()
    } // for()
    
    if ( ! hasFloatExist ) {
      ansNode -> lex = g.IntToStr( ( int ) ans ) ;
    } // if()
    else { // the final answer is a float
      ansNode -> lex = to_string( ans ) ;
    } // else()
    
    return ansNode ;
  } // ProcessMath()
  
  Node* ProcessCompare( string funcName, vector<Node*> argList ) {
    Node* ansNode = new Node ;
    ansNode -> lex = "#t" ;
    ansNode -> type = SPECIAL ;
    ansNode -> parent = NULL ;
    ansNode -> left = NULL ;
    ansNode -> right = NULL ;
    
    // check whether all the arguments are numbers
    for ( int i = 0 ; i < argList.size() ; i ++ ) {
      Node* currentAug = EvaluateSExp( argList[ i ] ) ;
      if ( currentAug != NULL && currentAug -> type == ATOM
          && ( g.IsINT( currentAug -> lex )
              || g.IsFLOAT( currentAug -> lex ) ) ) {
        argList[ i ] = EvaluateSExp( argList[ i ] ) ;
      } // if()
      else { // a non number atom exist
        throw IncorrectArgumentTypeException( funcName, currentAug -> lex ) ;
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
  
  Node* ProcessStringCompare( string funcName, vector<Node*> argList ) {
    Node* ansNode = new Node ;
    ansNode -> lex = "#t" ;
    ansNode -> type = SPECIAL ;
    ansNode -> parent = NULL ;
    ansNode -> left = NULL ;
    ansNode -> right = NULL ;
    
    // check whether all the arguments are numbers
    for ( int i = 0 ; i < argList.size() ; i ++ ) {
      Node* currentAug = EvaluateSExp( argList[ i ] ) ;
      if ( currentAug != NULL
           && currentAug -> type == ATOM && g.IsStr( currentAug -> lex ) ) {
        argList[ i ] = currentAug ;
      } // if()
      else {
        throw IncorrectArgumentTypeException( funcName, currentAug -> lex ) ;
      } // if()
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
      ansNode -> lex = '"' + ansStr + '"' ;
    } // if()
    
    return ansNode ;
  } // ProcessStringCompare()
  
  Node* ProcessCondOperation( string funcName, vector<Node*> argList ) {
    Node* ansNode = new Node ;
    ansNode -> lex = "#t" ;
    ansNode -> type = SPECIAL ;
    ansNode -> parent = NULL ;
    ansNode -> left = NULL ;
    ansNode -> right = NULL ;
    
    // check whether all the arguments are numbers
    for ( int i = 0 ; i < argList.size() ; i ++ ) {
      Node* currentAug = EvaluateSExp( argList[ i ] ) ;
      if ( currentAug != NULL ) {
        argList[ i ] = currentAug ;
      } // if()
      else {
        throw IncorrectArgumentTypeException( funcName, argList[ i ] -> lex ) ;
      } // if()
    } // for()
    
    bool resultIsTrue = true ;
    for ( int i = 0 ; i < argList.size() ; i ++ ) {
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
  
  Node* ProcessOperation( string funcName, Node* inTree ) {

    vector<Node*> argList = GetArgumentList( inTree ) ;
    
    if ( IsMathOperator( funcName ) ) { // need to have more than two arguments
      if ( CountArgument( inTree ) >= 2 ) {
        return ProcessMath( funcName, argList ) ;
      } // if()
      else {
        throw IncorrectNumberArgumentException( funcName ) ;
      } // else()
    } // if()
    else if ( IsComparison( funcName ) ) { // need to have more than two arguments
      if ( CountArgument( inTree ) >= 2 ) {
        return ProcessCompare( funcName, argList );
      } // if()
      else {
        throw IncorrectNumberArgumentException( funcName ) ;
      } // else()
    } // else if()
    else if ( IsCondOperator( funcName ) ) { // not only need 1 argument
      if ( funcName == "not" ) { // only ONE argument
        if ( CountArgument( inTree ) == 1 ) {
          return ProcessCondOperation( funcName, argList ) ;
        } // if()
        else {
          throw IncorrectNumberArgumentException( funcName ) ;
        } // else()
      } // if()
      else {
        if ( CountArgument( inTree ) >= 2 ) {
          return ProcessCondOperation( funcName, argList ) ;
        } // if()
        else {
          throw IncorrectNumberArgumentException( funcName ) ;
        } // else()
      } // else()
    } // else if()
    else if ( IsStringOperator( funcName ) ) {
      // need to have more than two arguments
      if ( CountArgument( inTree ) >= 2 ) {
        return ProcessStringCompare( funcName, argList ) ;
      } // if()
      else {
        throw IncorrectNumberArgumentException( funcName ) ;
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
  
  Node* ProcessEqvAndEqual( string funcName, Node* inTree ) {
    Node* ansNode = new Node ;
    ansNode -> lex = "#t" ;
    ansNode -> type = SPECIAL ;
    ansNode -> parent = NULL ;
    ansNode -> left = NULL ;
    ansNode -> right = NULL ;
    
    if ( CountArgument( inTree ) == 2 ) {
      vector<Node*> argList = GetArgumentList( inTree ) ;
      
      for ( int i = 0 ; i < argList.size() ; i ++ ) {
        EvaluateSExp( argList[ i ] ) ; // only used to check whether is any wrong
      } // for()
      
      if ( funcName == "eqv?" ) { // compare the pointer
        bool isInSameMemory = false ;
        
        if ( argList[ 0 ] -> type != CONS && argList[ 1 ] -> type != CONS
             && g.GetTokenType( argList[ 0 ] -> lex ) == SYMBOL
             && g.GetTokenType( argList[ 1 ] -> lex ) == SYMBOL ) {
          int symIndex1 = FindDefinedSymbol( argList[ 0 ] -> lex ) ;
          int symIndex2 = FindDefinedSymbol( argList[ 1 ] -> lex ) ;
          if ( mSymbolTable[ symIndex1 ].tree
               == mSymbolTable[ symIndex2 ].tree ) {
            isInSameMemory = true ;
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
          Node* tmp1 = EvaluateSExp( argList[ 0 ] ) ;
          Node* tmp2 = EvaluateSExp( argList[ 1 ] ) ;
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
        string originArgStr1 = argList[ 0 ] -> lex ;
        string originArgStr2 = argList[ 1 ] -> lex ;
        argList[ 0 ] = EvaluateSExp( argList[ 0 ] ) ;
        argList[ 1 ] = EvaluateSExp( argList[ 1 ] ) ;
        
        if ( argList[ 0 ] == NULL ) {
          throw IncorrectArgumentTypeException( funcName, originArgStr1 ) ;
        } // if()
        else if ( argList[ 1 ] == NULL ) {
          throw IncorrectArgumentTypeException( funcName, originArgStr2 ) ;
        } // else if()
        else {
          if ( ! TwoTreesAreTheSame( argList[ 0 ], argList[ 1 ] ) ) {
            ansNode -> lex = "nil" ;
          } // if()
        } // else()
      } // else if()
    } // if()
    else {
      throw IncorrectNumberArgumentException( funcName ) ;
    } // else()
    
    return ansNode ;
  } // ProcessEqvAndEqual()
  
  Node* ProcessIf( Node* inTree ) {
    Node* emptyNode = g.GetEmptyNode() ;
    // has two or three arguments
    if ( CountArgument( inTree ) == 2 || CountArgument( inTree ) == 3 ) {
      vector<Node*> argList = GetArgumentList( inTree ) ;
      
      // the first arguments should be the condition
      // if the evaluate of argment 1 is NULL then the format is wrong
      Node* condition = EvaluateSExp( argList[ 0 ] ) ;
      if ( condition != NULL ) {
        if ( CountArgument( inTree ) == 2 ) {
          if ( condition -> lex != "#f" && condition -> lex != "nil" ) {
            return EvaluateSExp( argList[ 1 ] ) ;
          } // if()
          else {
            throw NoReturnValueException( inTree ) ;
          } // else()
        } // if()
        else if ( CountArgument( inTree ) == 3 ) {
          if ( condition -> lex != "#f" && condition -> lex != "nil" ) {
            return EvaluateSExp( argList[ 1 ] ) ;
          } // if()
          else {
            return EvaluateSExp( argList[ 2 ] ) ;
          } // else()
        } // else if()
      } // if()
      else {
        throw IncorrectArgumentTypeException( "if", argList[ 0 ] -> lex ) ;
      } // else()
    } // if()
    else {
      throw IncorrectNumberArgumentException( "if" ) ;
    } // else()
    
    return emptyNode ;
  } // ProcessIf()
  
  Node* ProcessCond( Node* inTree ) {
    // cond expression cam take more than 1 arguments, at least one
    Node* emptyNode = g.GetEmptyNode() ;
    
    if ( CountArgument( inTree ) >= 1 ) {
      vector<Node*> argList = GetArgumentList( inTree ) ;
      
      // check each arguments should all be cons
      for ( int i = 0 ; i < argList.size() ; i ++ ) {
        if ( argList[ i ] -> type != CONS
             || ! IsList( argList[ i ], argList[ i ] ) ) {
          throw CondFormatException( inTree ) ;
        } // if()
      } // for()
      
      // start finding the first satisfying statement
      for ( int i = 0 ; i < argList.size() ; i ++ ) {
        Node* condResult = g.GetEmptyNode() ;
        Node* condPart = argList[ i ] -> left ;
        Node* statePart = g.GetEmptyNode() ;
        vector<Node*> subAugList = GetArgumentList( argList[ i ] ) ;
        
        for ( int subI = 0 ; subI < subAugList.size() ; subI ++ ) {
          if ( subI < subAugList.size() - 1 ) {
            EvaluateSExp( subAugList[ subI ] ) ;
          } // if()
          else {
            statePart = subAugList[ subI ] ;
          } // else()
        } // for()
        
        // assert: has deal with all the additional statement
        
        if ( i < argList.size() - 1 ) {
          condResult = EvaluateSExp( condPart ) ;
          if ( condResult != NULL ) {
            if ( condResult -> lex != "#f"
                && condResult -> lex != "nil"  ) {
              return EvaluateSExp( statePart ) ;
            } // if()
          } // if()
          else {
            throw CondFormatException( inTree ) ;
          } // else()
        } // if()
        else {
          // the last condition can start with the key word "else"
          if ( condPart -> lex == "else" ) { // don't need to evaluate
            return EvaluateSExp( statePart );
          } // if()
          else {
            condResult = EvaluateSExp( condPart ) ;
            if ( condResult != NULL ) {
              if ( condResult -> lex != "#f"
                  && condResult -> lex != "nil" ) {
                return EvaluateSExp( statePart ) ;
              } // if()
              else {
                throw NoReturnValueException( inTree ) ;
              } // else()
            } // if()
            else {
              throw CondFormatException( inTree ) ;
            } // else()
          } // else()
        } // else()
      } // for()
    } // if()
    else {
      throw CondFormatException( inTree ) ;
    } // else()
    
    return emptyNode ;
  } // ProcessCond()
  
  Node* ProcessBegin( Node* inTree ) {
    Node* emptyNode = g.GetEmptyNode() ;
    if ( CountArgument( inTree ) >= 1 ) {
      // sequencing evaluate all argements, but return the final one
      vector<Node*> argList = GetArgumentList( inTree ) ;
      
      for ( int i = 0 ; i < argList.size() ; i ++ ) {
        if ( !IsList( argList[ i ], argList[ i ] ) ) {
          throw IncorrectArgumentTypeException( "begin", argList[ i ] -> lex ) ;
        } // if()
      } // for()
      
      for ( int i = 0 ; i < argList.size() ; i ++ ) {
        if ( i == argList.size() - 1 ) {
          return EvaluateSExp( argList[ i ] ) ; ;
        } // if()
        else {
          EvaluateSExp( argList[ i ] ) ;
        } // else()
      } // for()
    } // if()
    else {
      throw IncorrectNumberArgumentException( "begin" ) ;
    } // else()
    
    return emptyNode ;
  } // ProcessBegin()
  
  void AddOriginReserveWords() {
    
    for ( int i = 0 ; i < 47 ; i ++ ) {
      Node* tmpNode = new Node ;
      tmpNode -> lex = "#<procedure " + g.GetStrContent( mOriginReserveWordList[ i ] ) + ">" ;
      tmpNode -> type = ATOM ;
      tmpNode -> parent = NULL ;
      tmpNode -> left = NULL ;
      tmpNode -> right = NULL ;
      
      Symbol tmpSym ;
      tmpSym.name = mOriginReserveWordList[ i ] ;
      tmpSym.tree = tmpNode ;
      
      mSymbolTable.push_back( tmpSym ) ;
    } // for()
  } // AddOriginReserveWords()
  
  string GetFuncNameFromFuncValue( string str ) {
    string name = "" ;
    int whiteIndex = ( int ) str.find( ' ', 0 ) ;
    for ( int i = whiteIndex + 1 ; i < str.length() ; i ++ ) {
      if ( str[ i ] != '>' ) {
        name += str[ i ] ;
      } // if()
    } // for()
    
    return name ;
  } // GetFuncNameFromFuncValue()
  
public:
  Evaluator() {
    AddOriginReserveWords() ;
    ResetReserveWord() ;
  } // Evaluator()
  
  Node* EvaluateSExp( Node* treeRoot ) {
    // the first left atom should be the func name
    Node* result = NULL ;
    string originFuncName = "" ;
    string funcName = "" ;
    int definedFuncIndex = -1 ;
    
    if ( treeRoot -> type != CONS ) {
      string reserveWord = GetReserveWordType( treeRoot -> lex ) ;
      if ( reserveWord != "" ) {
        return mSymbolTable[ FindDefinedSymbol( treeRoot -> lex ) ].tree ;
      } // if()
      else if ( g.GetTokenType( treeRoot -> lex ) == SYMBOL ) {
        int symbolIndex = FindDefinedSymbol( treeRoot -> lex ) ;
        if ( symbolIndex != -1 ) { // this symbol exist in the symbol table
          return EvaluateSExp( mSymbolTable[ symbolIndex ].tree ) ;
        } // if()
        else if ( treeRoot -> lex[ 0 ] == '#' ) { // the lex is start with #
          return treeRoot ; // this is an ATOM of the Reserve Word
        } // else if()
        else {
          throw UnboundValueException( treeRoot -> lex ) ;
        } // else()
      } // else if()
      
      return treeRoot ;
    } // if()
    else { // this S-exp is a cons
      // New observation: the function value can also process the S-exp
      if ( treeRoot -> left -> type == CONS ) {
        Node* funcNode = EvaluateSExp( treeRoot -> left ) ;
        originFuncName = funcNode -> lex ;
      } // if()
      else {
        originFuncName = treeRoot -> left -> lex ;
      } // else()
      
      if ( originFuncName[ 0 ] == '#' ) { // is a function value
        originFuncName = GetFuncNameFromFuncValue( originFuncName ) ;
      } // if()
      else {
        definedFuncIndex = FindDefinedFunc( originFuncName ) ;
      } // else()
    } // else()
    
    if ( IsList( treeRoot, treeRoot ) ) { // keep doing the evaluation
      funcName = GetReserveWordType( originFuncName ) ;
      if ( funcName != "" ) {
        if ( funcName == "quote" ) {
          return treeRoot -> right -> left ;
        } // if()
        else if ( funcName == "cons" ) {
          return EvaluateCONS( treeRoot ) ;
        } // else if()
        else if ( funcName == "list" ) {
          return EvaluateLIST( treeRoot ) ;
        } // else if()
        else if ( funcName == "define" ) {
          return Define( treeRoot ) ;
        } // else if()
        else if ( funcName == "car" || funcName == "cdr" ) {
          return AccessList( funcName, treeRoot ) ;
        } // else if()
        else if ( IsPredicator( funcName ) ) {
          return PrimitivePredecates( funcName, treeRoot ) ;
        } // else if()
        else if ( IsBasicOperation( funcName ) ) {
          return ProcessOperation( funcName, treeRoot );
        } // else if()
        else if ( funcName == "eqv?" || funcName == "equal?" ) {
          return ProcessEqvAndEqual( funcName, treeRoot );
        } // else if()
        else if ( funcName == "if" ) {
          return ProcessIf( treeRoot ) ;
        } // else if()
        else if ( funcName == "cond" ) {
          return ProcessCond( treeRoot ) ;
        } // else if()
        else if ( funcName == "begin" ) {
          return ProcessBegin( treeRoot ) ;
        } // else if()
        else if ( funcName == "clean-environment" ) {
          CleanEnvironment() ;
          return NULL ; // no tree to return
        } // else if()
        else if ( funcName == "exit" ) {
          if ( CountArgument( treeRoot ) != 0 ) {
            throw IncorrectNumberArgumentException( funcName ) ;
          } // if()
          
          return NULL ;
        } // else if()
      } // if()
      else if ( definedFuncIndex != -1 ) { // this user-defined function exist
        // process the user defined function
      } // else if()
      else if ( IsSymbol( originFuncName ) ) {
        if ( FindDefinedSymbol( originFuncName ) == -1 ) {
          throw UnboundValueException( originFuncName ) ;
        } // if()
        else {
          int index = FindDefinedSymbol( originFuncName ) ;
          throw ApplyNonFunctionException( originFuncName, mSymbolTable[ index ].tree ) ;
        } // else()
      } // else if()
      else { // either a function name or a symbol
        throw ApplyNonFunctionException( originFuncName, NULL ) ;
      } // else()
    } // if()
    else {
      throw NonListException( treeRoot ) ;
    } // else()
    
    return result ;
  } // EvaluateSExp()
  
} ; // Evaluator

// isList()

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
        if ( !gIsEOF ) {
          // g.PrettyPrint( tree.GetRoot() ) ; // proj.1
          try {
            Node* result = eval.EvaluateSExp( tree.GetRoot() ) ;
            if ( result != NULL ) {
              g.PrettyPrint( result ) ;
            } // if()
          } // try
          catch ( NonListException e ) {
            cout << e.Err_mesg() ;
            g.PrettyPrint( e.Err_node() ) ;
            cout << endl ;
          } // catch()
          catch ( UnboundValueException e ) {
            cout << e.Err_mesg() << endl ;
          } // catch()
          catch ( ApplyNonFunctionException e ) {
            cout << e.Err_mesg() ;
            e.PrintErrAtom() ;
          } // catch()
          catch ( IncorrectNumberArgumentException e ) {
            cout << e.Err_mesg() << endl ;
          } // catch()
          catch ( IncorrectArgumentTypeException e ) {
            cout << e.Err_mesg() << endl ;
          } // catch()
          catch ( NoReturnValueException e ) {
            cout << e.Err_mesg() ;
            g.PrettyPrint( e.Err_node() ) ;
            cout << endl ;
          } // catch()
          catch ( DivideByZeroException e ) {
            cout << e.Err_mesg() << endl ;
          } // catch()
          catch ( DefineFormatException e ) {
            cout << e.Err_mesg() ;
            g.PrettyPrint( e.Err_node() ) ;
            cout << endl ;
          } // catch()
          catch ( CondFormatException e ) {
            cout << e.Err_mesg() ;
            g.PrettyPrint( e.Err_node() ) ;
            cout << endl ;
          } // catch()
        } // if()
      } // if()
      
      gOriginalList.Clear() ;
      g.Reset() ;
      gJustFinishAExp = true ;
      
    } // catch()
    catch ( EOFException e ) {
      cout << e.Err_mesg() << endl ;
      gJustFinishAExp = true ;
      cout << "Thanks for using OurScheme!" << endl ;
      return 0 ;
    } // catch()
    catch ( MissingAtomOrLeftParException e ) {
      cout << e.Err_mesg() << endl ;
      gJustFinishAExp = true ;
    } // catch()
    catch ( MissingRightParException e ) {
      cout << e.Err_mesg() << endl ;
      gJustFinishAExp = true ;
    } // catch()
    catch ( NoClosingQuoteException e ) {
      cout << e.Err_mesg() << endl ;
      gJustFinishAExp = true ;
    } // catch()
    
  } // while()
  
  cout << endl << "Thanks for using OurScheme!" << endl ;
  
} // main()
