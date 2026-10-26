#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

namespace arcade {
constexpr int W=640,H=360,LastWave=6;
constexpr float Step=1.f/120.f;
using Color=uint32_t;
struct Vec { float x=0,y=0; };
struct Rect { float x,y,w,h; };
inline bool overlap(Rect a,Rect b) { return a.x<b.x+b.w&&a.x+a.w>b.x&&a.y<b.y+b.h&&a.y+a.h>b.y; }
inline float distance(Vec a,Vec b) { return std::hypot(a.x-b.x,a.y-b.y); }
inline Vec direction(Vec from,Vec to,float speed) { float d=distance(from,to); return d>.001f?Vec{(to.x-from.x)*speed/d,(to.y-from.y)*speed/d}:Vec{speed,0}; }
struct Input {
    bool left=false,right=false,jump=false,fire=false,dash=false,pulse=false;
    bool start=false,pause=false,restart=false,mute=false;
    int upgrade=0;
};
enum class Mode { Title,Playing,Paused,Upgrade,Won,GameOver };
enum class Sound { Shoot,Pickup,Dash,Hurt,Wave,Win };
enum class Kind { Slime,Wasp,Orb,Boss };
enum class Drop { Gem,Health,Pulse };
struct Player {
    Vec pos{305,280},vel{};
    bool grounded=false;
    int facing=1,hp=5,maxHP=5,power=2;
    float coyote=0,buffer=0,invincible=0,cooldown=0,shootFlash=0;
    float dash=0,dashCooldown=0,fireInterval=.23f;
    Rect box() const { return {pos.x,pos.y,18,30}; }
    Vec center() const { return {pos.x+9,pos.y+15}; }
};
struct Enemy {
    Vec pos,vel{};
    Kind kind=Kind::Slime;
    int hp=4,maxHP=4;
    float timer=0,age=0,spawn=.65f;
    bool alive=true;
    Rect box() const { return {pos.x,pos.y,kind==Kind::Boss?64.f:kind==Kind::Slime?25.f:23.f,kind==Kind::Boss?67.f:22.f}; }
    Vec center() const { auto b=box(); return {b.x+b.w/2,b.y+b.h/2}; }
};
struct Bullet { Vec p,v; float life=2.5f; int damage=1; bool hostile=false,alive=true; };
struct Pickup { Vec p,v; Drop kind=Drop::Gem; float life=18; bool alive=true; };
struct Particle { Vec p,v; float life,total; Color color; int size; };
struct Game {
    Player player;
    Mode mode=Mode::Title;
    std::vector<Rect> solids;
    std::vector<Enemy> enemies;
    std::vector<Bullet> bullets;
    std::vector<Pickup> pickups;
    std::vector<Particle> particles;
    std::vector<Sound> sounds;
    int wave=1,pending=0,spawned=0,kills=0,score=0,best=0,chain=0,pulses=2;
    float camera=0,time=0,elapsed=0,shake=0,banner=0,spawnTimer=0,comboTimer=0,pulseRing=0;
    bool muted=false;
    uint32_t random=0x47ab5u;
    Game();
    void reset();
    void beginWave(int number);
    void spawnEnemy(Kind kind,int side);
    void update(const Input&,float dt=Step);
    void hurt();
    void damageEnemy(Enemy&,int amount);
    void burst(Vec,Color,int count);
    void chooseUpgrade(int);
    int multiplier() const { return std::min(5,1+chain/4); }
    int aliveEnemies() const;
    void emit(Sound s) { if(!muted) sounds.push_back(s); }
};

struct Canvas {
    std::vector<Color> pixels;
    Canvas():pixels(W*H){}
    void clear(Color);
    void pixel(int,int,Color);
    void blend(int,int,Color,int);
    void rect(int,int,int,int,Color);
    void line(int,int,int,int,Color);
    void circle(int,int,int,Color);
    void ellipse(int,int,int,int,Color);
    void poly(const std::vector<Vec>&,Color);
    void text(int,int,const std::string&,Color,int scale=1);
    int textWidth(const std::string& s,int scale=1) const { return int(s.size())*6*scale; }
    void centered(int y,const std::string& s,Color c,int scale=1) { text((W-textWidth(s,scale))/2,y,s,c,scale); }
    void savePPM(const std::string&) const;
};
void render(const Game&,Canvas&);
} // namespace arcade
