#include "game.hpp"

namespace arcade {
Game::Game() { reset(); mode=Mode::Title; }
void Game::reset() {
    player=Player{}; score=kills=chain=0; pulses=2; elapsed=shake=comboTimer=pulseRing=0;
    bullets.clear(); pickups.clear(); particles.clear(); enemies.clear(); sounds.clear();
    random=0x47ab5u;
    solids={{0,310,640,75},{65,248,128,24},{251,204,135,24},{452,248,126,24}};
    mode=Mode::Playing; beginWave(1);
}
int Game::aliveEnemies() const { return int(std::count_if(enemies.begin(),enemies.end(),[](const Enemy& e){return e.alive;})); }
void Game::beginWave(int number) {
    wave=number; pending=4+number*2; spawned=0; spawnTimer=1.2f; banner=2.4f;
    bullets.clear(); pickups.clear(); enemies.clear();
    player.invincible=std::max(player.invincible,1.f);
    if(wave==LastWave) { pending=5; spawnEnemy(Kind::Boss,1); }
    emit(Sound::Wave);
}
void Game::spawnEnemy(Kind kind,int side) {
    Enemy e; e.kind=kind;
    e.pos={side>0?590.f:26.f,kind==Kind::Slime?288.f:108.f+float(spawned%3)*34};
    e.hp=kind==Kind::Slime?3+wave:kind==Kind::Wasp?2+wave:kind==Kind::Orb?5+wave:125;
    e.maxHP=e.hp; e.timer=1.3f;
    if(kind==Kind::Boss) { e.pos={524,243}; e.spawn=1.6f; }
    enemies.push_back(e);
}
void Game::burst(Vec p,Color c,int count) {
    for(int i=0;i<count;++i) {
        random^=random<<13; random^=random>>17; random^=random<<5;
        float life=.3f+float(random%45)/100;
        particles.push_back({p,{float(int(random%181)-90),-float(15+random%150)},life,life,c,1+int(random%3)});
    }
}
void Game::hurt() {
    if(mode!=Mode::Playing||player.invincible>0||player.dash>0) return;
    --player.hp; player.invincible=1.3f; chain=0; comboTimer=0; shake=.22f;
    burst(player.center(),0xef997f,16); emit(Sound::Hurt);
    if(player.hp<=0) { player.hp=0; mode=Mode::GameOver; best=std::max(best,score); }
}
void Game::damageEnemy(Enemy& e,int amount) {
    if(!e.alive||e.spawn>0) return;
    e.hp-=amount; burst(e.center(),0xa2d7b0,4);
    if(e.hp>0) return;
    e.alive=false; ++kills; ++chain; comboTimer=3.2f;
    int value=e.kind==Kind::Boss?3000:e.kind==Kind::Orb?220:e.kind==Kind::Wasp?150:100;
    score+=value*multiplier(); burst(e.center(),e.kind==Kind::Boss?0xffc86b:0x8ccc87,e.kind==Kind::Boss?65:15);
    pickups.push_back({e.center(),{0,-90},Drop::Gem,18,true});
    if(kills%5==0) pickups.push_back({e.center(),{35,-120},Drop::Health,20,true});
    if(kills%9==0) pickups.push_back({e.center(),{-35,-120},Drop::Pulse,20,true});
    shake=e.kind==Kind::Boss?.4f:.06f;
}
void Game::chooseUpgrade(int choice) {
    if(mode!=Mode::Upgrade||choice<1||choice>3) return;
    if(choice==1) ++player.power;
    if(choice==2) player.fireInterval=std::max(.085f,player.fireInterval*.78f);
    if(choice==3) { player.maxHP=std::min(10,player.maxHP+1); player.hp=player.maxHP; }
    mode=Mode::Playing; beginWave(wave+1);
}
void Game::update(const Input& in,float dt) {
    sounds.clear();
    if(!std::isfinite(dt)||dt<=0) return;
    dt=std::min(dt,.033f);
    if(in.mute) muted=!muted;
    if(mode==Mode::Title) { time+=dt; if(in.start||in.jump||in.fire) reset(); return; }
    if(in.restart) { reset(); return; }
    if(mode==Mode::Upgrade) { if(in.upgrade) chooseUpgrade(in.upgrade); else if(in.start) chooseUpgrade(1); return; }
    if(in.pause) { if(mode==Mode::Playing) mode=Mode::Paused; else if(mode==Mode::Paused) mode=Mode::Playing; }
    if(mode!=Mode::Playing) { if((mode==Mode::Won||mode==Mode::GameOver)&&in.start) reset(); return; }
    time+=dt; elapsed+=dt; banner=std::max(0.f,banner-dt); shake=std::max(0.f,shake-dt); pulseRing=std::max(0.f,pulseRing-dt);
    comboTimer=std::max(0.f,comboTimer-dt); if(comboTimer==0) chain=0;
    for(auto& fx:particles) { fx.life-=dt; fx.v.y+=200*dt; fx.p.x+=fx.v.x*dt; fx.p.y+=fx.v.y*dt; }
    particles.erase(std::remove_if(particles.begin(),particles.end(),[](const Particle& p){return p.life<=0;}),particles.end());

    auto& p=player;
    p.invincible=std::max(0.f,p.invincible-dt); p.cooldown=std::max(0.f,p.cooldown-dt); p.shootFlash=std::max(0.f,p.shootFlash-dt);
    p.dashCooldown=std::max(0.f,p.dashCooldown-dt); p.dash=std::max(0.f,p.dash-dt);
    p.buffer=in.jump?.12f:std::max(0.f,p.buffer-dt); p.coyote=p.grounded?.1f:std::max(0.f,p.coyote-dt);
    int dir=int(in.right)-int(in.left); if(dir) p.facing=dir;
    if(in.dash&&p.dashCooldown<=0) { p.dash=.16f; p.dashCooldown=1.3f; burst(p.center(),0x7cd8c1,14); emit(Sound::Dash); }
    if(p.dash>0) { p.vel.x=p.facing*490.f; p.vel.y=0; }
    else {
        p.vel.x+=std::clamp(dir*180.f-p.vel.x,-1500.f*dt,1500.f*dt);
        if(p.buffer>0&&p.coyote>0) { p.vel.y=-350; p.buffer=p.coyote=0; p.grounded=false; burst({p.pos.x+9,p.pos.y+29},0xadc5a0,6); }
        p.vel.y=std::min(650.f,p.vel.y+860*dt);
    }
    p.pos.x+=p.vel.x*dt;
    for(auto s:solids) if(overlap(p.box(),s)) { p.pos.x=p.vel.x>0?s.x-18:s.x+s.w; p.vel.x=0; }
    p.pos.x=std::clamp(p.pos.x,8.f,W-26.f);
    p.pos.y+=p.vel.y*dt; p.grounded=false;
    for(auto s:solids) if(overlap(p.box(),s)) { if(p.vel.y>=0) { p.pos.y=s.y-30; p.grounded=true; } else p.pos.y=s.y+s.h; p.vel.y=0; }
    if(p.pos.y>H) { p.pos={305,280}; p.invincible=0; hurt(); }

    spawnTimer-=dt;
    if(pending>0&&spawnTimer<=0) {
        Kind kind=spawned%3==2&&wave>=3?Kind::Orb:spawned%2==1&&wave>=2?Kind::Wasp:Kind::Slime;
        spawnEnemy(kind,spawned%2?1:-1); ++spawned; --pending; spawnTimer=std::max(.45f,1.f-wave*.07f);
    }
    if(in.fire&&p.cooldown<=0) {
        Vec origin{p.pos.x+9,p.pos.y+15},target{origin.x+p.facing*100,origin.y};
        float nearest=10000;
        for(const auto& e:enemies) if(e.alive&&e.spawn<=0) { float d=distance(origin,e.center()); if(d<nearest) { nearest=d; target=e.center(); } }
        Vec velocity=direction(origin,target,410);
        if(!dir) p.facing=velocity.x>=0?1:-1;
        bullets.push_back({origin,velocity,2.5f,p.power,false,true});
        p.cooldown=p.fireInterval; p.shootFlash=.10f; emit(Sound::Shoot);
    }
    if(in.pulse&&pulses>0&&pulseRing<=0) {
        --pulses; pulseRing=.65f; shake=.18f; burst(p.center(),0xffcf7b,45); emit(Sound::Dash);
        for(auto& e:enemies) if(e.alive&&distance(p.center(),e.center())<170) damageEnemy(e,12);
        for(auto& b:bullets) if(b.hostile&&distance(p.center(),b.p)<190) b.alive=false;
    }
    for(auto& e:enemies) if(e.alive) {
        e.age+=dt; e.spawn=std::max(0.f,e.spawn-dt); if(e.spawn>0) continue;
        float dx=p.center().x-e.center().x;
        if(e.kind==Kind::Slime) {
            e.pos.x+=(dx>0?1:-1)*(32+wave*3)*dt;
            e.vel.y=std::min(500.f,e.vel.y+860*dt); e.pos.y+=e.vel.y*dt;
            for(auto s:solids) if(overlap(e.box(),s)&&e.vel.y>=0) { e.pos.y=s.y-22; e.vel.y=0; }
        } else if(e.kind==Kind::Wasp) {
            Vec v=direction(e.center(),p.center(),32+wave*4.f); e.pos.x+=v.x*dt; e.pos.y+=(v.y+std::sin(e.age*5)*22)*dt;
            e.pos.y=std::clamp(e.pos.y,70.f,280.f);
        } else if(e.kind==Kind::Orb) {
            e.pos.x+=(std::abs(dx)>200?(dx>0?1:-1)*24.f:0.f)*dt;
            e.pos.y+=(std::sin(e.age*3)*15)*dt;
            e.timer-=dt; if(e.timer<=0) { bullets.push_back({e.center(),direction(e.center(),p.center(),125),5,1,true,true}); e.timer=2.2f-wave*.13f; }
        } else {
            e.pos.x+=(dx>0?1:-1)*19*dt; e.pos.x=std::clamp(e.pos.x,18.f,558.f);
            e.timer-=dt;
            if(e.timer<=0) {
                Vec v=direction(e.center(),p.center(),140); float angle=std::atan2(v.y,v.x);
                for(int k=-1;k<=1;++k) { float a=angle+k*.23f; bullets.push_back({e.center(),{std::cos(a)*140,std::sin(a)*140},5,1,true,true}); }
                if(e.hp<e.maxHP/2) for(int k=0;k<8;++k) { float a=k*.785398f+e.age; bullets.push_back({e.center(),{std::cos(a)*85,std::sin(a)*85},5,1,true,true}); }
                e.timer=e.hp<e.maxHP/2?.95f:1.4f;
            }
        }
        if(overlap(p.box(),e.box())) hurt();
    }
    for(auto& b:bullets) if(b.alive) {
        b.life-=dt; b.p.x+=b.v.x*dt; b.p.y+=b.v.y*dt;
        if(b.life<=0||b.p.x< -24||b.p.x>W+24||b.p.y<40||b.p.y>H+24) { b.alive=false; continue; }
        Rect hit{b.p.x-3,b.p.y-3,6,6};
        if(b.hostile) { if(overlap(hit,p.box())) { hurt(); b.alive=false; } }
        else for(auto& e:enemies) if(e.alive&&e.spawn<=0&&overlap(hit,e.box())) { damageEnemy(e,b.damage); b.alive=false; break; }
    }
    bullets.erase(std::remove_if(bullets.begin(),bullets.end(),[](const Bullet& b){return !b.alive;}),bullets.end());
    for(auto& drop:pickups) if(drop.alive) {
        drop.life-=dt; drop.v.y=std::min(160.f,drop.v.y+240*dt); drop.p.x+=drop.v.x*dt; drop.p.y+=drop.v.y*dt;
        if(drop.p.y>299) { drop.p.y=299; drop.v={0,0}; }
        if(distance(drop.p,p.center())<78) { Vec v=direction(drop.p,p.center(),180); drop.p.x+=v.x*dt; drop.p.y+=v.y*dt; }
        if(distance(drop.p,p.center())<17) {
            if(drop.kind==Drop::Gem) score+=25*multiplier();
            if(drop.kind==Drop::Health) p.hp=std::min(p.maxHP,p.hp+1);
            if(drop.kind==Drop::Pulse) pulses=std::min(5,pulses+1);
            drop.alive=false; burst(drop.p,0xffcf7b,6); emit(Sound::Pickup);
        }
        if(drop.life<=0) drop.alive=false;
    }
    pickups.erase(std::remove_if(pickups.begin(),pickups.end(),[](const Pickup& p){return !p.alive;}),pickups.end());
    if(mode==Mode::Playing&&pending==0&&aliveEnemies()==0) {
        // Collect outstanding rewards before the upgrade screen removes the arena drops.
        for(auto& drop:pickups) if(drop.alive) {
            if(drop.kind==Drop::Gem) score+=25*multiplier();
            if(drop.kind==Drop::Health) p.hp=std::min(p.maxHP,p.hp+1);
            if(drop.kind==Drop::Pulse) pulses=std::min(5,pulses+1);
        }
        pickups.clear(); bullets.clear(); score+=wave*300;
        if(wave==LastWave) { score+=p.hp*500; mode=Mode::Won; best=std::max(best,score); emit(Sound::Win); }
        else { mode=Mode::Upgrade; emit(Sound::Wave); }
    }
    // Compact dead entities so long sessions do not retain obsolete state.
    enemies.erase(std::remove_if(enemies.begin(),enemies.end(),[](const Enemy& e){return !e.alive;}),enemies.end());
}
} // namespace arcade
