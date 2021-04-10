
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
  Node* errNode ;
public:
  NonListException( Node* errTree ) {
    errNode = errTree ;
  } // NonListException()
  
  string Err_mesg() {
    string mesg = "ERROR (non-list): " ;
    return mesg ;
  } // Err_mesg()
  
  Node* Err_node() {
    return errNode ;
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
  
public:
  ApplyNonFunctionException( string errLex ) {
    mLex = errLex ;
  } // ApplyNonFunctionException()
  
  string Err_mesg() {
    string mesg = "ERROR (attempt to apply non-function) : " + mLex ;
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

// --------------------- Error Definition Proj.2 (end) ---------------------

// Purpose: Do the evaluation and store the user definitions
class Evaluator {
private:
  string mReserveWord[ 34 ] = { "cons", "list", "quote", "define", "car", "cdr", "not", "and", "or", "begin", "if", "cond", "clean-environment", "quote", "'", "atom?", "pair?", "list?", "null?", "integer?", "real?", "number?", "string?", "boolean?", "symbol?", "+", "-", "*", "\\", "eqv?", "equal?", "begin", "if", "cond" } ;
  
  struct Symbol {
    string name ;
    Node* tree ;
  } ; //SymbolInfo
  
  struct Function {
    string name ;
    int augNum ;
    Node* tree ;
  } ; // Function
  
  vector<Symbol> mSymbolTable ;
  vector<Function> mFunctionTable ;
  
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
  
  bool IsReserveWord( string str ) {
    if ( str == "cons" || str == "list" || str == "quote" || str == "define" || str == "car" || str == "cdr" || str == "not" || str == "and" || str == "or" || str == "begin" || str == "if" || str == "cond" || str == "clean-environment" || str == "quote" || str == "'" || str == "atom?" || str == "pair?" || str == "list?" || str == "null?" || str == "integer?" || str == "real?" || str == "number?" || str == "string?" || str == "boolean?" || str == "symbol?" || str == "+" || str == "-" || str == "*" || str == "\\" || str == "eqv?" || str == "equal?" || str == "begin" || str == "if" || str == "cond" ) {
      return true ;
    } // if()
    
    return false ;
  } // IsReserveWord()
  
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
    vector<Node*> augList ;
    if ( CountArgument( inTree ) > 0 ) {
      // Step1. find out all the arguments and put them in a list
      augList = GetArgumentList( inTree )  ;
      // Step2. start to check the type and mean time replace the symbol
      for ( int i = 0 ; i < augList.size() ; i ++ ) {
        if ( IsList( augList[ i ], augList[ i ] ) ) {
          if ( augList[ i ] -> type == ATOM
              && IsSymbol( augList[ i ] -> lex ) ) {
            int symIndex = FindDefinedSymbol( augList[ i ] -> lex ) ;
            if ( symIndex != -1 ) {
              augList[ i ] = EvaluateSExp( mSymbolTable[ symIndex ].tree ) ;
            } // if()
            else {
              throw UnboundValueException( augList[ i ] -> lex ) ;
            } // else()
          } // if()
          else { // this is a list, but need to check more detail
            augList[ i ] = EvaluateSExp( augList[ i ] ) ;
          } // else()
        } // if()
        else {
          throw NonListException( augList[ i ] ) ;
        } // else()
      } // for()
      // Step3. All the arguments are correct, now combined them
      Node* prevNode = NULL ;
      for ( int i = 0 ; i < augList.size() ; i ++ ) {
        Node* node = new Node ;
        node -> lex = "" ;
        node -> type = CONS ;
        node -> parent = NULL ;
        node -> left = NULL ;
        node -> right = NULL ;
        
        node -> left = augList[ i ] ;
        
        if ( i == 0 ) {
          result = node ;
          prevNode = node ;
        } // if()
        else if ( i == augList.size() - 1 ){
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
  
  void Define( Node* inTree ) {
    vector<Node*> augList = GetArgumentList( inTree ) ;
    
    if ( CountArgument( inTree ) == 2 ) {
      // the first argument should be a symbol
      if ( augList[ 0 ] -> type == ATOM ) {
        if ( g.GetTokenType( augList[ 0 ] -> lex ) == SYMBOL ) {
          Node* value = augList[ 1 ] ; // copy
          
          int symIndex = FindDefinedSymbol( augList[ 0 ] -> lex ) ;
          Symbol newSymbol ;
          newSymbol.name = augList[ 0 ] -> lex ;
          newSymbol.tree = value ;
          if ( symIndex != -1 ) { // this symbol has already exist, update it
            mSymbolTable[ symIndex ].tree = value ;
          } // if()
          else {
            mSymbolTable.push_back( newSymbol ) ;
          } // else()
          
          cout << newSymbol.name << " defined" << endl ;
        } // if()
        else {
          throw IncorrectArgumentTypeException( "define", augList[ 0 ] -> lex ) ;
        } // else()
      } // if()
      else { //
        throw IncorrectArgumentTypeException( "define", augList[ 0 ] -> left -> lex ) ;
      } // else()
    } // if()
    else {
      throw IncorrectNumberArgumentException( "define" ) ;
    } // else()
  } // Define()
  
public:
  
  Node* EvaluateSExp( Node* treeRoot ) {
    // the first left atom should be the func name
    Node* result = NULL ;
    string funcName = "" ;
    int definedFuncIndex = -1 ;
    
    if ( treeRoot -> type == CONS ) {
      funcName = treeRoot -> left -> lex ;
      definedFuncIndex = FindDefinedFunc( funcName ) ;
    } // if()
    
    if ( treeRoot -> type != CONS ) {
      if ( g.GetTokenType( treeRoot -> lex ) == SYMBOL ) {
        int symbolIndex = FindDefinedSymbol( treeRoot -> lex ) ;
        if ( symbolIndex != -1 ) { // this symbol exist in the symbol table
          return EvaluateSExp( mSymbolTable[ symbolIndex ].tree ) ;
        } // if()
        else {
          throw UnboundValueException( treeRoot -> lex ) ;
        } // else()
      } // if()
      
      return treeRoot ;
    } // if()
    
    if ( IsList( treeRoot, treeRoot ) ) { // keep doing the evaluation
      if ( IsReserveWord( funcName ) ) {
        if ( funcName == "quote" ) {
          // g.PrettyPrint( treeRoot -> right -> left ) ;
          return treeRoot -> right -> left ;
        } // if()
        else if ( funcName == "cons" ) {
          Node* result = EvaluateCONS( treeRoot ) ;
          // g.PrettyPrint( result ) ;
          return result ;
        } // else if()
        else if ( funcName == "list" ) {
          Node* result = EvaluateLIST( treeRoot ) ;
          // g.PrettyPrint( result ) ;
          return result ;
        } // else if()
        else if ( funcName == "define" ) {
          Define( treeRoot ) ;
          return NULL ;
        } // else if()
      } // if()
      else if ( definedFuncIndex != -1 ) { // this user-defined function exist
        // process the user defined function
      } // else if()
      else {
        if ( ! g.IsINT( funcName ) && ! g.IsFLOAT( funcName )
             && FindDefinedSymbol( funcName ) == -1 ) {
          throw UnboundValueException( funcName ) ;
        } // if()
        else {
          throw ApplyNonFunctionException( funcName ) ;
        } // else()
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
            cout << e.Err_mesg() << endl ;
          } // catch()
          catch ( IncorrectNumberArgumentException e ) {
            cout << e.Err_mesg() << endl ;
          } // catch()
          catch ( IncorrectArgumentTypeException e ) {
            cout << e.Err_mesg() << endl ;
          } // catch()
          catch ( NoReturnValueException e ) {
            cout << e.Err_mesg() ;
            g.PrettyPrint( tree.GetRoot() ) ;
            cout << endl ;
          } // catch()
          catch ( DivideByZeroException e ) {
            cout << e.Err_mesg() << endl ;
          } // catch()
          catch ( DefineFormatException e ) {
            cout << e.Err_mesg() ;
            g.PrettyPrint( tree.GetRoot() ) ;
            cout << endl ;
          } // catch()
          catch ( CondFormatException e ) {
            cout << e.Err_mesg() ;
            g.PrettyPrint( tree.GetRoot() ) ;
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
