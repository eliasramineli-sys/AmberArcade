#include "game.hpp"
#include <iostream>
#include <limits>
#include <stdexcept>
using namespace arcade;
int count=0;
void check(bool ok,const char* label) { if(!ok) throw std::runtime_error(label); ++count; std::cout<<"PASS "<<label<<'\n'; }
void tick(Game& g,int frames,Input input={}) { for(int i=0;i<frames;++i) g.update(input); }
void isolated(Game& g) { g.reset(); g.spawnTimer=1000; }
int main() {
    try {
        Game g; check(g.mode==Mode::Title,"title on launch");
        Input in; in.start=true; g.update(in); tick(g,40); check(g.mode==Mode::Playing&&g.player.grounded,"start lands in arena");
        float x=g.player.pos.x; in={}; in.right=true; tick(g,40,in); check(g.player.pos.x>x+45,"accelerated movement");
        in={}; in.jump=true; float y=g.player.pos.y; g.update(in); tick(g,15); check(g.player.pos.y<y-25,"jump leaves ground");
        isolated(g); g.player.pos.x=9; in={}; in.left=true; tick(g,40,in); check(g.player.pos.x==8,"arena clamps player");
        isolated(g); in={}; in.fire=true; tick(g,90,in); check(!g.bullets.empty()&&g.bullets.size()<8,"held fire and cooldown");
        isolated(g); g.spawnEnemy(Kind::Wasp,1); g.enemies[0].pos={370,280}; g.enemies[0].spawn=0;
        in={}; in.fire=true; tick(g,90,in); check(g.kills==1&&g.score>=150,"automatic aim and projectile damage");
        isolated(g); g.player.invincible=0; g.hurt(); check(g.player.hp==4,"damage reduces health");
        g.hurt(); check(g.player.hp==4,"temporary invincibility");
        g.player.invincible=0; in={}; in.dash=true; g.update(in); x=g.player.pos.x; g.hurt(); tick(g,5);
        check(g.player.hp==4&&g.player.pos.x>x+15,"dash movement and damage immunity");
        g.update(in); check(g.player.dashCooldown>1,"dash cannot bypass cooldown");
        isolated(g); g.player.invincible=0; g.bullets.push_back({g.player.center(),{},1,1,true,true}); g.update({});
        check(g.player.hp==4&&g.bullets.empty(),"hostile shot hits player once");
        isolated(g); g.spawnEnemy(Kind::Slime,1); g.enemies[0].pos={340,288}; g.enemies[0].spawn=0;
        in={}; in.pulse=true; g.update(in); check(g.kills==1&&g.pulses==1,"pulse damages nearby enemies and consumes charge");
        g.update(in); check(g.pulses==1,"pulse cooldown");
        isolated(g); g.pulses=0; in={}; in.pulse=true; g.update(in); check(g.pulseRing==0&&g.pulses==0,"empty pulse cannot activate");
        isolated(g); g.player.hp=2; g.pickups.push_back({g.player.center(),{},Drop::Health,10,true});
        g.pickups.push_back({g.player.center(),{},Drop::Pulse,10,true}); g.update({}); check(g.player.hp==3&&g.pulses==3,"health and pulse pickups");
        isolated(g); g.pickups.push_back({{50,100},{},Drop::Gem,.001f,true}); g.update({}); check(g.pickups.empty(),"expired pickup removed");
        isolated(g);
        for(int i=0;i<4;++i) { Enemy e; e.hp=1; e.spawn=0; g.damageEnemy(e,1); }
        check(g.multiplier()==2,"consecutive kills increase combo"); tick(g,400); check(g.multiplier()==1&&g.chain==0,"combo expires without kills");
        isolated(g); g.pending=1; g.update({}); check(g.mode==Mode::Playing,"pending spawns block wave completion");
        g.pending=0; g.update({}); check(g.mode==Mode::Upgrade,"cleared wave opens upgrade screen");
        g.chooseUpgrade(9); check(g.mode==Mode::Upgrade,"invalid upgrade rejected");
        g.chooseUpgrade(1); check(g.wave==2&&g.player.power==3&&g.pending==8,"power upgrade starts next wave");
        g.mode=Mode::Upgrade; float rate=g.player.fireInterval; g.chooseUpgrade(2); check(g.player.fireInterval<rate&&g.wave==3,"rate upgrade");
        g.mode=Mode::Upgrade; g.player.hp=1; g.chooseUpgrade(3); check(g.player.maxHP==6&&g.player.hp==6,"health upgrade heals fully");
        isolated(g); in={}; in.pause=true; g.update(in); float elapsed=g.elapsed; in={}; in.fire=true; tick(g,120,in);
        check(g.mode==Mode::Paused&&g.elapsed==elapsed&&g.bullets.empty(),"pause freezes simulation");
        in={}; in.pause=true; g.update(in); check(g.mode==Mode::Playing,"pause resumes");
        in={}; in.mute=true; g.update(in); check(g.muted,"mute setting");
        g.beginWave(LastWave); check(g.enemies.size()==1&&g.enemies[0].kind==Kind::Boss,"final wave spawns guardian");
        g.enemies[0].spawn=0; g.player.invincible=100; tick(g,170);
        check(std::any_of(g.bullets.begin(),g.bullets.end(),[](const Bullet& b){return b.hostile;}),"guardian fires hostile projectiles");
        g.pending=0; for(auto& e:g.enemies) { e.spawn=0; g.damageEnemy(e,10000); } g.update({});
        check(g.mode==Mode::Won&&g.best==g.score,"guardian defeat finishes sixth wave");
        int best=g.best; g.reset(); check(g.best==best&&g.muted&&g.wave==1,"restart preserves record and settings");
        isolated(g); g.player.invincible=0;
        for(int i=0;i<5;++i) { g.player.invincible=0; g.hurt(); }
        check(g.mode==Mode::GameOver&&g.player.hp==0,"zero health ends game");
        float clock=g.time; g.update({},std::numeric_limits<float>::quiet_NaN()); check(g.time==clock,"invalid delta time ignored");
        Canvas c;
        for(auto mode:{Mode::Title,Mode::Playing,Mode::Paused,Mode::Upgrade,Mode::Won,Mode::GameOver}) { g.mode=mode; render(g,c); }
        check(c.pixels.size()==size_t(W*H),"every game screen renders");
        c.rect(-90,-90,120,120,0x123456); c.poly({{-20,-20},{-50,450},{900,100}},0xabcdef); check(c.pixels[0]==0xabcdef,"offscreen draw clipping");

        Game run; run.reset();
        for(int frame=0;frame<120*240&&(run.mode==Mode::Playing||run.mode==Mode::Upgrade);++frame) {
            Input bot; bot.fire=true;
            int phase=(frame/240)%2; bot.right=phase==0; bot.left=!bot.right;
            bot.jump=frame%90==0; bot.dash=frame%165==0;
            bot.pulse=run.aliveEnemies()>=3&&run.pulseRing==0;
            bot.upgrade=run.wave==4?3:1;
            run.update(bot);
        }
        std::cout<<"Full run wave="<<run.wave<<" health="<<run.player.hp<<" kills="<<run.kills<<" score="<<run.score<<" time="<<run.elapsed<<'\n';
        check(run.mode==Mode::Won,"automated gameplay wins all six waves");
        std::cout<<count<<" checks passed\n";
        return 0;
    } catch(const std::exception& e) { std::cerr<<"FAIL "<<e.what()<<'\n'; return 1; }
}
