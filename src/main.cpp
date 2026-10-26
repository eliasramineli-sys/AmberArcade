#include "game.hpp"
#include <exception>
#include <filesystem>
#include <iostream>
#ifdef _WIN32
int runWindows();
#endif

namespace {
void preview(arcade::Game& g) {
    using namespace arcade;
    g.reset(); g.wave=3; g.pending=7; g.time=9.2f; g.elapsed=43; g.banner=0;
    g.score=8250; g.best=12400; g.chain=9; g.comboTimer=2.6f;
    g.player.invincible=0; g.player.pos={312,166}; g.player.vel={120,-30}; g.player.shootFlash=.1f;
    g.enemies.clear();
    for(auto k:{Kind::Slime,Kind::Wasp,Kind::Orb,Kind::Slime,Kind::Wasp}) g.spawnEnemy(k,1);
    g.enemies[0].pos={155,288}; g.enemies[1].pos={238,119}; g.enemies[2].pos={519,127};
    g.enemies[3].pos={472,288}; g.enemies[4].pos={80,149};
    for(auto& e:g.enemies) e.spawn=0;
    g.bullets={{{374,170},{350,-45},1,2,false,true},{{431,156},{350,-45},1,2,false,true},
               {{471,176},{-95,52},1,1,true,true},{{416,224},{-95,52},1,1,true,true}};
    g.pickups={{{185,274},{},Drop::Gem,10,true},{{430,291},{},Drop::Health,10,true}};
    g.burst({333,183},0x8ce8cc,14); g.burst({150,291},0xffcf7b,12);
}
}
int main(int argc,char** argv) {
    try {
        if(argc>=3&&std::string(argv[1])=="--screenshot") {
            arcade::Game g; std::string scene=argc>3?argv[3]:"play";
            if(scene=="title") { g.time=3; }
            else { preview(g); if(scene=="upgrade") g.mode=arcade::Mode::Upgrade; if(scene=="boss") { g.wave=6; g.enemies.clear(); g.spawnEnemy(arcade::Kind::Boss,1); g.enemies[0].spawn=0; g.enemies[0].hp=74; } }
            arcade::Canvas c; arcade::render(g,c); c.savePPM(argv[2]);
            std::cout<<"Rendered "<<argv[2]<<'\n'; return 0;
        }
        if(argc>=3&&std::string(argv[1])=="--demo") {
            std::filesystem::create_directories(argv[2]);
            arcade::Game g; g.reset(); arcade::Canvas c;
            for(int i=0;i<960;++i) {
                arcade::Input in; in.fire=true; in.right=(i/240)%2==0; in.left=!in.right;
                in.jump=i%95==0; in.dash=i%260==0; in.pulse=i%350==300;
                if(g.mode==arcade::Mode::Upgrade) in.upgrade=1;
                g.update(in);
                if(i%6==0) { arcade::render(g,c); std::string n=std::to_string(10000+i/6).substr(1); c.savePPM(std::string(argv[2])+"/frame"+n+".ppm"); }
            }
            std::cout<<"Demo rendered: 160 frames at 20 FPS\n"; return 0;
        }
#ifdef _WIN32
        return runWindows();
#else
        std::cout<<"Amber Arcade: interactive play requires Windows.\n"
                    "Headless capture: ./AmberArcade --screenshot image.ppm [title|play|boss|upgrade]\n";
        return 0;
#endif
    } catch(const std::exception& e) { std::cerr<<e.what()<<'\n'; return 1; }
}
