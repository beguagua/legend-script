#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <unordered_map>
#include <variant>
#include <vector>

#include "opengl_backend.hpp"

namespace fs = std::filesystem;
namespace legend {
struct Vec2 { double x=0,y=0; };
struct Vec3 { double x=0,y=0,z=0; };
struct Quat { double x=0,y=0,z=0,w=1; };
struct Color { double r=1,g=1,b=1,a=1; };
struct Transform { Vec3 position{}; Quat rotation{}; Vec3 scale{1,1,1}; };
struct Entity { int id=0; std::string name; Transform transform{}; std::vector<std::string> components; };
struct Wait { double seconds=0; };
using Value = std::variant<std::monostate,double,std::string,bool,Vec2,Vec3,Quat,Color,Transform,Entity,Wait>;

static std::string number(double n){ std::ostringstream o; o<<n; return o.str(); }
static std::string text(const Value& v){
  if(std::holds_alternative<std::monostate>(v)) return "nil";
  if(auto p=std::get_if<double>(&v)) return number(*p);
  if(auto p=std::get_if<std::string>(&v)) return *p;
  if(auto p=std::get_if<bool>(&v)) return *p?"true":"false";
  if(auto p=std::get_if<Vec2>(&v)) return "vec2("+number(p->x)+", "+number(p->y)+")";
  if(auto p=std::get_if<Vec3>(&v)) return "vec3("+number(p->x)+", "+number(p->y)+", "+number(p->z)+")";
  if(auto p=std::get_if<Quat>(&v)) return "quat("+number(p->x)+", "+number(p->y)+", "+number(p->z)+", "+number(p->w)+")";
  if(auto p=std::get_if<Color>(&v)) return "color("+number(p->r)+", "+number(p->g)+", "+number(p->b)+", "+number(p->a)+")";
  if(auto p=std::get_if<Transform>(&v)) return "transform(position="+text(p->position)+", scale="+text(p->scale)+")";
  if(auto p=std::get_if<Entity>(&v)) return "entity("+number(p->id)+", "+p->name+")";
  return "wait("+number(std::get<Wait>(v).seconds)+")";
}
static double num(const Value& v){if(auto p=std::get_if<double>(&v))return *p;throw std::runtime_error("expected number, got "+text(v));}
static bool truth(const Value& v){if(auto p=std::get_if<bool>(&v))return *p;if(auto p=std::get_if<double>(&v))return *p!=0;return !std::holds_alternative<std::monostate>(v);}
static Vec3 vec3(const Value& v){if(auto p=std::get_if<Vec3>(&v))return *p;throw std::runtime_error("expected vec3");}

enum class Kind {End,Number,String,Identifier,Plus,Minus,Star,Slash,LParen,RParen,Equal,Semicolon,Comma,EqualEqual,BangEqual,Greater,Less,GreaterEqual,LessEqual};
struct Token{Kind kind;std::string lexeme;double value=0;int line=1;};
class Lexer{std::string s;size_t a=0,b=0;int line=1;public:explicit Lexer(std::string x):s(std::move(x)){}std::vector<Token> scan(){std::vector<Token> t;while(b<s.size()){a=b;one(t);}t.push_back({Kind::End,"",0,line});return t;}private:char peek(){return b<s.size()?s[b]:'\0';}char adv(){return s[b++];}void add(std::vector<Token>&t,Kind k){t.push_back({k,s.substr(a,b-a),0,line});}void one(std::vector<Token>&t){char c=adv();if(std::isspace((unsigned char)c)){if(c=='\n')line++;return;}if(c=='/'&&peek()=='/'){while(peek()!='\n'&&peek())adv();return;}if(std::isdigit((unsigned char)c)){while(std::isdigit((unsigned char)peek()))adv();if(peek()=='.'){adv();while(std::isdigit((unsigned char)peek()))adv();}Token x{Kind::Number,s.substr(a,b-a),std::stod(s.substr(a,b-a)),line};t.push_back(x);return;}if(std::isalpha((unsigned char)c)||c=='_'){while(std::isalnum((unsigned char)peek())||peek()=='_')adv();add(t,Kind::Identifier);return;}if(c=='"'){std::string x;while(peek()!='"'&&peek()){char q=adv();if(q=='\\'&&peek()){char e=adv();x+=e=='n'?'\n':e;}else x+=q;}if(!peek())throw std::runtime_error("unterminated string");adv();t.push_back({Kind::String,x,0,line});return;}switch(c){case '+':add(t,Kind::Plus);break;case '-':add(t,Kind::Minus);break;case '*':add(t,Kind::Star);break;case '/':add(t,Kind::Slash);break;case '(':add(t,Kind::LParen);break;case ')':add(t,Kind::RParen);break;case ';':add(t,Kind::Semicolon);break;case ',':add(t,Kind::Comma);break;case '=':add(t,peek()=='='?(adv(),Kind::EqualEqual):Kind::Equal);break;case '!':add(t,peek()=='='?(adv(),Kind::BangEqual):Kind::BangEqual);break;case '>':add(t,peek()=='='?(adv(),Kind::GreaterEqual):Kind::Greater);break;case '<':add(t,peek()=='='?(adv(),Kind::LessEqual):Kind::Less);break;default:throw std::runtime_error("unexpected character: "+std::string(1,c));}}};

struct Runtime { int next_entity=1;std::vector<Entity> entities;std::vector<Wait> coroutines;std::string active_scene="main";double game_time=0;bool running=true; };
class Parser{std::vector<Token> t;size_t p=0;std::unordered_map<std::string,Value>& env;Runtime& rt;public:Parser(std::vector<Token>x,std::unordered_map<std::string,Value>&e,Runtime&r):t(std::move(x)),env(e),rt(r){}void run(){while(!check(Kind::End)){statement();match(Kind::Semicolon);}}private:
 bool check(Kind k){return t[p].kind==k;}bool match(Kind k){if(check(k)){p++;return true;}return false;}Token eat(Kind k,const char*m){if(!check(k))throw std::runtime_error(m);return t[p++];}
 void statement(){if(check(Kind::Identifier)&&(t[p].lexeme=="auto"||t[p].lexeme=="let"||t[p].lexeme=="var")){p++;auto n=eat(Kind::Identifier,"expected variable name");eat(Kind::Equal,"expected '='");env[n.lexeme]=expression();return;}if(check(Kind::Identifier)&&(t[p].lexeme=="output"||t[p].lexeme=="out"||t[p].lexeme=="print")){p++;eat(Kind::LParen,"expected '('");auto v=expression();eat(Kind::RParen,"expected ')'");std::cout<<text(v)<<'\n';return;}if(check(Kind::Identifier)&&t[p].lexeme=="yield"){p++;auto v=expression();rt.coroutines.push_back(Wait{num(v)});return;}expression();}
 Value expression(){return equality();}Value equality(){auto a=comparison();while(check(Kind::EqualEqual)||check(Kind::BangEqual)){auto op=t[p++].kind;auto b=comparison();bool x=text(a)==text(b);a=op==Kind::EqualEqual?Value(x):Value(!x);}return a;}Value comparison(){auto a=term();while(check(Kind::Greater)||check(Kind::Less)||check(Kind::GreaterEqual)||check(Kind::LessEqual)){auto op=t[p++].kind;double x=num(a),y=num(term());a=op==Kind::Greater?Value(x>y):op==Kind::Less?Value(x<y):op==Kind::GreaterEqual?Value(x>=y):Value(x<=y);}return a;}Value term(){auto a=factor();while(check(Kind::Plus)||check(Kind::Minus)){auto op=t[p++].kind;auto b=factor();if(op==Kind::Plus&&(std::holds_alternative<std::string>(a)||std::holds_alternative<std::string>(b)))a=text(a)+text(b);else a=op==Kind::Plus?Value(num(a)+num(b)):Value(num(a)-num(b));}return a;}Value factor(){auto a=unary();while(check(Kind::Star)||check(Kind::Slash)){auto op=t[p++].kind;double b=num(unary());a=op==Kind::Star?Value(num(a)*b):Value(num(a)/b);}return a;}Value unary(){if(match(Kind::Minus))return -num(unary());return primary();}
 Value primary(){if(match(Kind::Number))return t[p-1].value;if(match(Kind::String))return t[p-1].lexeme;if(match(Kind::LParen)){auto v=expression();eat(Kind::RParen,"expected ')'");return v;}if(match(Kind::Identifier)){std::string n=t[p-1].lexeme;if(n=="true")return true;if(n=="false")return false;if(n=="nil")return {};if(match(Kind::LParen)){std::vector<Value>a;if(!check(Kind::RParen)){do{a.push_back(expression());}while(match(Kind::Comma));}eat(Kind::RParen,"expected ')'");return call(n,a);}if(auto i=env.find(n);i!=env.end())return i->second;throw std::runtime_error("unknown identifier: "+n);}throw std::runtime_error("expected expression");}
 Value call(const std::string&n,const std::vector<Value>&a){auto need=[&](size_t x){if(a.size()!=x)throw std::runtime_error(n+" expects "+std::to_string(x)+" arguments");};if(n=="vec2"){need(2);return Vec2{num(a[0]),num(a[1])};}if(n=="vec3"){need(3);return Vec3{num(a[0]),num(a[1]),num(a[2])};}if(n=="quat"){need(4);return Quat{num(a[0]),num(a[1]),num(a[2]),num(a[3])};}if(n=="color"){if(a.size()==3)return Color{num(a[0]),num(a[1]),num(a[2]),1};need(4);return Color{num(a[0]),num(a[1]),num(a[2]),num(a[3])};}if(n=="sqrt"){need(1);return std::sqrt(num(a[0]));}if(n=="spawn"){need(1);Entity e{rt.next_entity++,text(a[0])};rt.entities.push_back(e);return e;}if(n=="add_component"){need(2);auto e=std::get<Entity>(a[0]);e.components.push_back(text(a[1]));for(auto&x:rt.entities)if(x.id==e.id)x=e;return e;}if(n=="transform"){need(3);return Transform{vec3(a[0]),std::get<Quat>(a[1]),vec3(a[2])};}if(n=="set_transform"){need(2);auto e=std::get<Entity>(a[0]);auto tr=std::get<Transform>(a[1]);e.transform=tr;for(auto&x:rt.entities)if(x.id==e.id)x=e;return e;}if(n=="translate"){need(2);auto e=std::get<Entity>(a[0]);auto d=vec3(a[1]);e.transform.position.x+=d.x;e.transform.position.y+=d.y;e.transform.position.z+=d.z;for(auto&x:rt.entities)if(x.id==e.id)x=e;return e;}if(n=="wait"){need(1);return Wait{num(a[0])};}
if(n=="entity_count"){need(0);return double(rt.entities.size());}
if(n=="scene"){need(1);rt.active_scene=text(a[0]);return rt.active_scene;}
if(n=="current_scene"){need(0);return rt.active_scene;}
if(n=="time"){need(0);return rt.game_time;}
if(n=="delta_time"){need(0);return 1.0/60.0;}
if(n=="input_pressed"){need(1);return false;}
if(n=="input_axis"){need(1);return 0.0;}
if(n=="play_sound"){need(1);std::cout<<"[audio] play "<<text(a[0])<<'\n';return {};}
if(n=="draw_mesh"){need(3);std::cout<<"[render] mesh="<<text(a[0])<<" entity="<<text(a[1])<<" material="<<text(a[2])<<'\n';return {};}
if(n=="collides"){need(2);auto x=std::get<Entity>(a[0]);auto y=std::get<Entity>(a[1]);auto d=x.transform.position;auto e=y.transform.position;double dx=d.x-e.x,dy=d.y-e.y,dz=d.z-e.z;return dx*dx+dy*dy+dz*dz<1.0;}
if(n=="log"){need(1);std::cerr<<"[legend] "<<text(a[0])<<'\n';return {};}
if(n=="quit"){need(0);rt.running=false;return {};}
throw std::runtime_error("unknown function: "+n);}
};
static void execute(const std::string&s,std::unordered_map<std::string,Value>&e,Runtime&r){Parser(Lexer(s).scan(),e,r).run();}
static std::string read_file(const fs::path&p){std::ifstream f(p);if(!f)throw std::runtime_error("file not found: "+p.string());std::stringstream b;b<<f.rdbuf();return b.str();}
static void write_bytecode(const fs::path&out,const std::string&s){std::ofstream f(out,std::ios::binary);f<<"LGND\1"<<s;}
static std::string read_bytecode(const fs::path&p){std::ifstream f(p,std::ios::binary);std::string h(5,'\0');f.read(h.data(),5);if(h.substr(0,4)!="LGND")throw std::runtime_error("invalid Legend bytecode");std::stringstream b;b<<f.rdbuf();return b.str();}
}
int main(int argc,char**argv){using namespace legend;std::unordered_map<std::string,Value> env;Runtime rt;try{if(argc>1&&std::string(argv[1])=="--version"){std::cout<<"Legend Script v0.4.0\n";return 0;}if(argc>1&&std::string(argv[1])=="--opengl-version"){std::string requested=argv[2];auto dot=requested.find('.');if(dot==std::string::npos)throw std::runtime_error("OpenGL version must be MAJOR.MINOR");int major=std::stoi(requested.substr(0,dot)),minor=std::stoi(requested.substr(dot+1));bool core=argc>3&&std::string(argv[3])=="core";auto gl=graphics::OpenGLBackend::negotiate(major,minor,core);std::cout<<gl.name()<<" ("<<graphics::OpenGLBackend::profile_name(gl.core_profile)<<")\n";return 0;}if(argc>1&&std::string(argv[1])=="--help"){std::cout<<"Usage:\n  lgnd file.lgnd\n  lgnd -e '<script>'\n  lgnd --compile file.lgnd [out.lbc]\n  lgnd --bytecode file.lbc\n  lgnd --watch file.lgnd\n";return 0;}if(argc>1&&std::string(argv[1])=="-e"){execute(argv[2],env,rt);return 0;}if(argc>1&&std::string(argv[1])=="--compile"){fs::path in=argv[2],out=argc>3?argv[3]:in.replace_extension(".lbc");write_bytecode(out,read_file(in));std::cout<<"compiled "<<in<<" -> "<<out<<'\n';return 0;}if(argc>1&&std::string(argv[1])=="--bytecode"){execute(read_bytecode(argv[2]),env,rt);return 0;}if(argc>1&&std::string(argv[1])=="--run"){execute(read_file(argv[2]),env,rt);while(rt.running&&!rt.coroutines.empty()){auto w=rt.coroutines.back();rt.coroutines.pop_back();rt.game_time+=w.seconds;}return 0;}if(argc>1&&std::string(argv[1])=="--watch"){fs::path file=argv[2];auto stamp=fs::last_write_time(file);std::cout<<"hot reload watching "<<file<<"\n";for(;;){auto now=fs::last_write_time(file);if(now!=stamp){stamp=now;env.clear();rt=Runtime{};try{execute(read_file(file),env,rt);std::cout<<"reloaded\n";}catch(const std::exception&e){std::cerr<<"Legend error: "<<e.what()<<'\n';}}std::this_thread::sleep_for(std::chrono::milliseconds(200));} }if(argc>1){fs::path p=argv[1];if(p.extension()!=".lgnd")throw std::runtime_error("Legend files must use .lgnd");execute(read_file(p),env,rt);return 0;}std::cout<<"Legend Script v0.4.0\nlgnd> ";std::string line;while(std::getline(std::cin,line)&&line!="exit"){execute(line,env,rt);std::cout<<"lgnd> ";}}catch(const std::exception&e){std::cerr<<"Legend error: "<<e.what()<<'\n';return 1;}}
