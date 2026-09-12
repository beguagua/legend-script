#include <cctype>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace legend {
using Value = std::variant<std::monostate, double, std::string, bool>;

static std::string text(const Value& v) {
  if (std::holds_alternative<std::monostate>(v)) return "nil";
  if (auto n = std::get_if<double>(&v)) { std::ostringstream out; out << *n; return out.str(); }
  if (auto s = std::get_if<std::string>(&v)) return *s;
  return std::get<bool>(v) ? "true" : "false";
}
static bool truthy(const Value& v) {
  if (std::holds_alternative<std::monostate>(v)) return false;
  if (auto b = std::get_if<bool>(&v)) return *b;
  if (auto n = std::get_if<double>(&v)) return *n != 0;
  return !std::get<std::string>(v).empty();
}
static double number(const Value& v) {
  if (auto n = std::get_if<double>(&v)) return *n;
  throw std::runtime_error("expected a number, got '" + text(v) + "'");
}

enum class Kind { End, Number, String, Identifier, Plus, Minus, Star, Slash, LParen, RParen, Equal, Semicolon, Comma, BangEqual, EqualEqual, Greater, Less, GreaterEqual, LessEqual };
struct Token { Kind kind; std::string lexeme; double number = 0; int line = 1; };

class Lexer {
 public:
  explicit Lexer(std::string source) : source_(std::move(source)) {}
  std::vector<Token> scan() {
    std::vector<Token> out;
    while (!at_end()) { start_ = current_; scan_one(out); }
    out.push_back({Kind::End, "", 0, line_}); return out;
  }
 private:
  std::string source_; size_t start_=0, current_=0; int line_=1;
  bool at_end() const { return current_ >= source_.size(); }
  char advance() { return source_[current_++]; }
  char peek() const { return at_end() ? '\0' : source_[current_]; }
  char next() const { return current_ + 1 >= source_.size() ? '\0' : source_[current_+1]; }
  void add(std::vector<Token>& o, Kind k) { o.push_back({k, source_.substr(start_, current_-start_), 0, line_}); }
  void scan_one(std::vector<Token>& o) {
    char c=advance();
    if (std::isspace((unsigned char)c)) { if(c=='\n') ++line_; return; }
    if (c=='/' && peek()=='/') { while(peek()!='\n'&&!at_end()) advance(); return; }
    if (std::isdigit((unsigned char)c)) { while(std::isdigit((unsigned char)peek())) advance(); if(peek()=='.'){advance(); while(std::isdigit((unsigned char)peek()))advance();} Token t{Kind::Number,source_.substr(start_,current_-start_),0,line_}; t.number=std::stod(t.lexeme); o.push_back(t); return; }
    if (std::isalpha((unsigned char)c)||c=='_') { while(std::isalnum((unsigned char)peek())||peek()=='_') advance(); add(o,Kind::Identifier); return; }
    if (c=='"') { std::string s; while(peek()!='"'&&!at_end()){ if(advance()=='\\'&&!at_end()){char e=advance(); s += e=='n'?'\n':e;} else s += source_[current_-1]; } if(at_end()) throw std::runtime_error("unterminated string"); advance(); o.push_back({Kind::String,s,0,line_}); return; }
    switch(c){ case '+':add(o,Kind::Plus);break; case '-':add(o,Kind::Minus);break; case '*':add(o,Kind::Star);break; case '/':add(o,Kind::Slash);break; case '(':add(o,Kind::LParen);break; case ')':add(o,Kind::RParen);break; case ';':add(o,Kind::Semicolon);break; case ',':add(o,Kind::Comma);break; case '!': add(o,peek()=='='?(advance(),Kind::BangEqual):Kind::BangEqual);break; case '=':add(o,peek()=='='?(advance(),Kind::EqualEqual):Kind::Equal);break; case '>':add(o,peek()=='='?(advance(),Kind::GreaterEqual):Kind::Greater);break; case '<':add(o,peek()=='='?(advance(),Kind::LessEqual):Kind::Less);break; default: throw std::runtime_error("unexpected character: "+std::string(1,c)); }
  }
};

class Parser {
 public:
  Parser(std::vector<Token> t, std::unordered_map<std::string,Value>& e):tokens_(std::move(t)),env_(e){}
  void run() { while(!check(Kind::End)){ statement(); match(Kind::Semicolon); } }
 private:
  std::vector<Token> tokens_; size_t pos_=0; std::unordered_map<std::string,Value>& env_;
  bool check(Kind k) const{return tokens_[pos_].kind==k;} bool match(Kind k){if(check(k)){++pos_;return true;}return false;}
  Token consume(Kind k,const char* msg){if(!check(k)) throw std::runtime_error(msg);return tokens_[pos_++];}
  void statement(){
    if(check(Kind::Identifier) && (tokens_[pos_].lexeme=="auto"||tokens_[pos_].lexeme=="let"||tokens_[pos_].lexeme=="var")){++pos_; auto name=consume(Kind::Identifier,"expected variable name"); consume(Kind::Equal,"expected '=' after variable name"); env_[name.lexeme]=expression(); return;}
    if(check(Kind::Identifier) && (tokens_[pos_].lexeme=="output" || tokens_[pos_].lexeme=="out" || tokens_[pos_].lexeme=="print")){++pos_; consume(Kind::LParen,"expected '('"); Value v=expression(); consume(Kind::RParen,"expected ')' "); std::cout<<text(v)<<'\n'; return;}
    if(check(Kind::Identifier)&&tokens_[pos_].lexeme=="if"){++pos_; consume(Kind::LParen,"expected '('"); bool condition=truthy(expression()); consume(Kind::RParen,"expected ')'"); consume(Kind::Identifier,"expected 'then'"); if(!condition){ if(check(Kind::Identifier)&&tokens_[pos_].lexeme=="output") { while(!check(Kind::Semicolon)&&!check(Kind::End)) ++pos_; } } else statement(); return; }
    expression();
  }
  Value expression(){return equality();}
  Value equality(){Value a=comparison();while(check(Kind::EqualEqual)||check(Kind::BangEqual)){Kind op=tokens_[pos_++].kind;Value b=comparison();bool same=text(a)==text(b);a=op==Kind::EqualEqual?same:!same;}return a;}
  Value comparison(){Value a=term();while(check(Kind::Greater)||check(Kind::Less)||check(Kind::GreaterEqual)||check(Kind::LessEqual)){Kind op=tokens_[pos_++].kind;double x=number(a),y=number(term());a=op==Kind::Greater?x>y:op==Kind::Less?x<y:op==Kind::GreaterEqual?x>=y:x<=y;}return a;}
  Value term(){Value a=factor();while(check(Kind::Plus)||check(Kind::Minus)){Kind op=tokens_[pos_++].kind;Value b=factor();if(op==Kind::Plus&&(std::holds_alternative<std::string>(a)||std::holds_alternative<std::string>(b)))a=text(a)+text(b);else a=op==Kind::Plus?number(a)+number(b):number(a)-number(b);}return a;}
  Value factor(){Value a=unary();while(check(Kind::Star)||check(Kind::Slash)){Kind op=tokens_[pos_++].kind;double b=number(unary());a=op==Kind::Star?number(a)*b:number(a)/b;}return a;}
  Value unary(){if(match(Kind::Minus))return -number(unary());return primary();}
  Value primary(){if(match(Kind::Number))return tokens_[pos_-1].number;if(match(Kind::String))return tokens_[pos_-1].lexeme;if(match(Kind::LParen)){Value v=expression();consume(Kind::RParen,"expected ')'");return v;}if(match(Kind::Identifier)){auto n=tokens_[pos_-1].lexeme;if(n=="true")return true;if(n=="false")return false;if(n=="nil")return {};if(n=="sqrt"){consume(Kind::LParen,"expected '('");double v=number(expression());consume(Kind::RParen,"expected ')'");return std::sqrt(v);}if(auto it=env_.find(n);it!=env_.end())return it->second;throw std::runtime_error("unknown identifier: "+n);}throw std::runtime_error("expected expression");}
};

static void execute(const std::string& source, std::unordered_map<std::string,Value>& env){Parser(Lexer(source).scan(),env).run();}
}

int main(int argc,char** argv){
  using namespace legend; std::unordered_map<std::string,Value> env;
  try {
    if(argc>=2 && std::string(argv[1])=="--version"){std::cout<<"Legend Script v0.1.0\n";return 0;}
    if(argc>=2 && std::string(argv[1])=="--help"){std::cout<<"Usage:\n  legend [filename].lgnd\n  legend -e '<script>'\n  legend\n";return 0;}
    if(argc>=3 && std::string(argv[1])=="-e"){execute(argv[2],env);return 0;}
    if(argc>=2){std::string path=argv[1];if(path.size()<5||path.substr(path.size()-5)!=".lgnd") throw std::runtime_error("Legend files must use the .lgnd extension");std::ifstream in(path);if(!in) throw std::runtime_error("file not found: "+path);std::stringstream buf;buf<<in.rdbuf();execute(buf.str(),env);return 0;}
    std::cout<<"Legend Script v0.1.0\nlegend> "; std::string line;while(std::getline(std::cin,line)&&line!="exit"){execute(line,env);std::cout<<"legend> ";}
  } catch(const std::exception& e){std::cerr<<"Legend error: "<<e.what()<<'\n';return 1;}
}
