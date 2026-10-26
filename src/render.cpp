#include "game.hpp"
#include <array>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <cctype>
#include <stdexcept>
namespace arcade {
namespace {
constexpr Color ink=0x142c35,cream=0xffefd0,gold=0xffc86b;
int hash(int x,int y=0) { uint32_t z=uint32_t(x)*374761393u+uint32_t(y)*668265263u; z=(z^(z>>13))*1274126177u; return int((z^(z>>16))&0x7fffffff); }
Color mix(Color a,Color b,float t) {
    auto ch=[&](int n){return int(((a>>n)&255)*(1-t)+((b>>n)&255)*t);};
    return Color((ch(16)<<16)|(ch(8)<<8)|ch(0));
}
std::string number(int n,int digits=0) { std::ostringstream s; s<<std::setw(digits)<<std::setfill('0')<<n; return s.str(); }
void panel(Canvas& c,int x,int y,int w,int h) {
    c.rect(x+3,y+4,w,h,0x102b31); c.rect(x,y,w,h,0x547f78); c.rect(x+1,y+1,w-2,h-2,0x152f36);
    c.line(x+2,y+2,x+w-3,y+2,0x325750);
}
void star(Canvas& c,int x,int y,Color color,int r=3) { c.line(x-r,y,x+r,y,color); c.line(x,y-r,x,y+r,color); c.pixel(x+1,y+1,0xffebc0); }
void tree(Canvas& c,int x,int y,int scale,int seed,bool distant=false) {
    Color dark=distant?0x28595a:0x254b39,mid=distant?0x326660:0x3e7647,light=distant?0x3d7469:0x65a056;
    c.poly({{float(x-5*scale),float(y)},{float(x-2*scale),float(y-79*scale)},{float(x+5*scale),float(y-74*scale)},{float(x+8*scale),float(y)}},distant?0x2e5551:0x604e3e);
    c.rect(x,y-73*scale,2*scale,73*scale,distant?0x35605a:0x92704a);
    for(int pass=0;pass<3;++pass) for(int i=0;i<18;++i) {
        int dx=(hash(seed,i)%61-30)*scale,dy=(hash(seed+20,i)%47-23)*scale;
        int r=(13+hash(seed+100,i)%13)*scale;
        if(pass==0) c.circle(x+dx,y-80*scale+dy,r,dark);
        if(pass==1) c.circle(x+dx-2*scale,y-83*scale+dy,r-2*scale,mid);
        if(pass==2&&i%3==0) c.ellipse(x+dx-4*scale,y-88*scale+dy,(r-5*scale),std::max(1,(r-5*scale)/2),light);
    }
    if(!distant) for(int i=0;i<32;++i) { int dx=(hash(seed+90,i)%63-32)*scale,dy=(hash(seed+80,i)%53-27)*scale; c.rect(x+dx,y-85*scale+dy,2*scale,scale,0xacc578); }
}
void background(const Game& g,Canvas& c) {
    for(int y=0;y<H;++y) c.rect(0,y,W,1,y<230?mix(0x194f6b,0xb6cba5,float(y)/230):mix(0xb6cba5,0x315e5a,float(y-230)/130));
    int sunX=474-int(g.camera*.025f);
    for(int r=65;r>28;r-=5) { int alpha=3; for(int yy=-r;yy<=r;++yy) for(int xx=-r;xx<=r;++xx) if(xx*xx+yy*yy<r*r) c.blend(sunX+xx,90+yy,0xffd793,alpha); }
    c.circle(sunX,90,28,0xe3d598); c.circle(sunX-4,85,22,0xf0dda2);
    for(int i=-1;i<7;++i) {
        int x=i*150-int(g.camera*.06f)%150,y=48+hash(i+40)%62;
        c.rect(x,y,90,3,0xa0bbb0); c.rect(x+19,y-3,76,3,0xb0c4b2); c.rect(x+41,y-6,40,3,0xb0c4b2);
    }
    for(int layer=0;layer<3;++layer) {
        float speed=.10f+layer*.10f; int spacing=190-layer*25,base=222+layer*31;
        Color col=layer==0?0x558b8e:layer==1?0x396e76:0x28575e;
        for(int i=-2;i<8;++i) {
            int idx=int(g.camera*speed)/spacing+i,x=idx*spacing-int(g.camera*speed);
            int height=75+hash(idx+layer*50)%55;
            c.poly({{float(x-80),float(base)},{float(x+30),float(base-height)},{float(x+55),float(base-height+18)},{float(x+140),float(base)},{float(x+160),float(H)},{float(x-80),float(H)}},col);
            if(layer==0) c.poly({{float(x+4),float(base-height+24)},{float(x+30),float(base-height)},{float(x+55),float(base-height+18)},{float(x+38),float(base-height+15)},{float(x+28),float(base-height+11)}},0x9cbbaf);
        }
    }
    for(int i=-2;i<12;++i) {
        int idx=int(g.camera*.35f)/85+i,x=idx*85-int(g.camera*.35f);
        tree(c,x,333+(hash(idx+8)%20),1,idx+30,true);
    }
    for(int i=-1;i<6;++i) {
        int idx=int(g.camera*.58f)/190+i,x=idx*190-int(g.camera*.58f);
        tree(c,x,333,1,idx+180);
    }
    // A distant waterfall, animated in world space.
    int wx=1395-int(g.camera*.38f);
    if(wx>-30&&wx<W+30) { c.rect(wx,164,18,160,0x5d9e9d); c.rect(wx+3,164,4,160,0x9ec7b5); for(int i=0;i<12;++i) c.rect(wx+8,169+(i*17+int(g.time*24))%155,6,9,0x81b5ac); }
    for(int y=258;y<304;y+=3) for(int x=0;x<W;x+=3) c.blend(x,y,0xb6ccb1,5);
}
void terrain(Canvas& c,Rect s,int camera) {
    int x=int(s.x)-camera,y=int(s.y),w=int(s.w),h=int(s.h);
    if(x>W+24||x+w< -24) return;
    c.rect(x+3,y+7,w,h,0x18332f);
    c.rect(x,y+6,w,h-6,0x463f36);
    for(int row=0;row<(h+18)/18;++row) for(int col=0;col<(w+23)/24;++col) {
        int gx=int(s.x)/24+col,xx=x+col*24+(row%2?-10:0),yy=y+9+row*18;
        int variation=hash(gx,row)%4;
        const Color shades[]={0x78533b,0x8b6241,0x704d37,0x967046};
        int left=std::max(x,xx+1),right=std::min(x+w-1,xx+23);
        int bottom=std::min(y+h,yy+17);
        if(right<=left||bottom<=yy) continue;
        c.rect(left+1,yy+1,right-left-2,bottom-yy-2,shades[variation]);
        c.rect(left+3,yy,right-left-6,1,shades[variation]);
        c.rect(left,yy+3,1,std::max(0,bottom-yy-6),shades[variation]);
        c.line(left+2,yy,right-3,yy,0x9a7950);
        c.line(left,yy+1,left,bottom-2,0x8d6e48);
        c.line(left+2,bottom-2,right-2,bottom-2,0x4e4638);
        if(right-left>10&&bottom-yy>10) { c.rect(left+5,yy+5,4,2,0x9a7950); c.pixel(left+12,yy+8,0x534734); }
    }
    c.rect(x,y,w,5,0x254e38); c.rect(x+1,y,w-2,3,0x92bc5e); c.rect(x+2,y-2,w-4,2,0xc1d97b);
    for(int i=0;i<w;i+=6) {
        int depth=4+hash(int(s.x)+i)%9;
        c.rect(x+i,y+3,5,depth,0x3d7138); c.rect(x+i,y+3,2,depth-2,0x7ba449);
        if(hash(i+int(s.x),12)%3==0) { c.line(x+i,y-2,x+i-2,y-5,0x85b561); c.line(x+i+1,y-1,x+i+3,y-6,0xa2c476); }
    }
    if(s.y<300) {
        for(int i=10;i<w-7;i+=21) {
            int len=8+hash(i+int(s.x))%19;
            c.line(x+i,y+h,x+i-2,y+h+len,0x3a6747);
            c.rect(x+i-4,y+h+len/2,4,2,0x799958); c.rect(x+i-1,y+h+len-4,4,2,0x517e4b);
        }
    }
}
void hero(Canvas& c,int x,int y,const Game& g) {
    const auto& p=g.player;
    if(p.invincible>0&&int(g.time*18)%2) return;
    bool running=std::abs(p.vel.x)>20&&p.grounded;
    int stride=running?int(std::sin(g.time*17)*4):0;
    int bob=running?int(std::sin(g.time*34)>0):int(std::sin(g.time*3)>0);
    y-=bob;
    auto r=[&](int a,int b,int w,int h,Color col) { c.rect(x+(p.facing>0?a:18-a-w),y+b,w,h,col); };
    // Scarf, boots, tunic, hair and satchel use a limited hand-selected palette.
    r(-5,9,11,5,0x592d32); r(-8-int(std::sin(g.time*12)*2),11,8,3,0xcb654b); r(-7,10,6,1,0xf29968);
    r(4+stride,24,6,6,ink); r(11-stride,23,6,7,ink);
    r(4+stride,25,5,3,0x634c43); r(10-stride,24,5,4,0x96734d); r(5+stride,29,7,2,0x1a3035); r(10-stride,29,8,2,0x1a3035);
    r(3,13,14,12,ink); r(4,13,12,10,0x287e83); r(5,14,4,8,0x5db4ac); r(12,15,4,8,0x23595e);
    r(3,22,14,3,0x7e4f35); r(10,22,3,3,gold); r(4,21,2,2,0xb79862);
    r(1,15,5,7,0x18444e); r(0,20,5,4,0xe1ac73); r(1,20,3,2,0xffd596);
    r(13,14,6,7,0x215e67); r(15,20,5,4,0xe8b47c); r(15,20,4,2,0xffd596);
    r(3,3,14,12,ink); r(4,4,12,10,0xf4c38a); r(5,5,9,3,0xffdb9e); r(14,8,5,4,0xf4c38a);
    r(13,7,2,3,0x203a3b); r(14,7,1,1,cream); r(10,12,5,1,0xa86547);
    r(2,2,14,5,0x44352e); r(4,0,9,4,0x44352e); r(1,4,5,7,0x44352e); r(2,3,5,3,0x916245); r(5,1,7,2,0xb2814c);
    r(3,6,14,3,0xd27649); r(3,6,14,1,0xffbf6c); r(3,9,3,3,0x7d5038);
    r(-1,14,5,8,0x5b4b35); r(-1,14,4,2,0xab8a50);
    int gunX=x+(p.facing>0?18:-6);
    c.rect(gunX,y+16,9,5,ink); c.rect(gunX+1,y+17,7,2,0x9dc8b3);
    c.rect(gunX+(p.facing>0?7:-2),y+16,3,4,0xffcf7b);
    if(p.shootFlash>0) {
        star(c,gunX+(p.facing>0?12:-4),y+17,0x9ff8d4,5);
        c.circle(gunX+(p.facing>0?12:-4),y+17,2,0xffffff);
    }
    if(p.dash>0) for(int i=0;i<4;++i) c.rect(x-p.facing*(8+i*7),y+9+i*4,6,2,0x81dbc4);
}
void slime(Canvas& c,int x,int y,float time,float phase) {
    int b=int(std::sin(time*6+phase)*2);
    c.ellipse(x+13,y+18,16,3,0x1b3735);
    c.ellipse(x+12,y+11+b,13,10-b,ink); c.ellipse(x+12,y+9+b,12,9-b,0x438254); c.ellipse(x+10,y+7+b,9,7-b,0x77b56a);
    c.rect(x+4,y+4+b,4,2,0xb6d57e); c.rect(x+16,y+4+b,2,1,0x9fc979);
    c.rect(x+6,y+9+b,4,6,cream); c.rect(x+16,y+9+b,4,6,cream); c.rect(x+8,y+11+b,2,3,ink); c.rect(x+18,y+11+b,2,3,ink);
    c.rect(x+11,y+16,4,1,0x254637); c.rect(x+2,y+17,5,2,0x658f54); c.rect(x+20,y+17,5,2,0x658f54);
}

void enemyArt(Canvas& c,const Enemy& e,float t) {
    int x=int(e.pos.x),y=int(e.pos.y);
    if(e.spawn>0&&int(t*14)%2) return;
    if(e.kind==Kind::Slime) { slime(c,x,y+3,t,e.pos.x); }
    else if(e.kind==Kind::Wasp) {
        int flap=int(std::sin(t*25+x)*5);
        c.ellipse(x+3,y+4-flap,9,4,0x335e65); c.ellipse(x+20,y+4-flap,9,4,0x335e65);
        c.ellipse(x+3,y+2-flap,8,3,0xc4ddd0); c.ellipse(x+21,y+2-flap,8,3,0xc4ddd0);
        c.ellipse(x+12,y+12,13,9,ink); c.ellipse(x+12,y+11,11,7,0xc68e49);
        c.ellipse(x+10,y+8,8,4,0xf5cf7a); c.rect(x+4,y+10,4,8,0x61563c); c.rect(x+13,y+11,3,7,0x61563c);
        c.rect(x+17,y+9,4,4,cream); c.rect(x+18,y+10,2,3,ink);
        c.line(x+4,y+5,x+1,y+1,ink); c.line(x+20,y+5,x+23,y+1,ink);
        c.poly({{float(x+9),float(y+19)},{float(x+14),float(y+19)},{float(x+12),float(y+24)}},0xdabc73);
    } else if(e.kind==Kind::Orb) {
        y+=int(std::sin(t*4+x)*2);
        c.poly({{float(x+11),float(y-6)},{float(x+26),float(y+10)},{float(x+11),float(y+27)},{float(x-5),float(y+10)}},ink);
        c.poly({{float(x+11),float(y-3)},{float(x+23),float(y+10)},{float(x+11),float(y+23)},{float(x-2),float(y+10)}},0xab884c);
        c.poly({{float(x+11),float(y)},{float(x+20),float(y+10)},{float(x+11),float(y+20)},{float(x+1),float(y+10)}},0xdec27c);
        c.rect(x+5,y+5,13,12,0x2c5a5a); c.rect(x+7,y+7,9,8,0x6dc2b0); c.rect(x+9,y+8,5,5,0xd5f3cd);
        star(c,x+11,y+10,cream,2); c.rect(x+10,y-5,3,4,0xffe1a1);
    } else {
        // Moss guardian: stone shoulders, engraved chest and amber core.
        int bob=int(std::sin(t*3)*2);
        c.ellipse(x+32,y+67,42,5,0x193b36);
        c.rect(x+7,y+47,20,19,ink); c.rect(x+38,y+47,20,19,ink);
        c.rect(x+9,y+46,16,17,0x697766); c.rect(x+40,y+46,16,17,0x566c5e);
        c.rect(x+6,y+61,23,7,0x3d564b); c.rect(x+36,y+61,24,7,0x3d564b);
        c.rect(x+11,y+48,5,10,0xa1aa80); c.rect(x+42,y+48,4,10,0x8d9b75);
        c.poly({{float(x+6),float(y+18+bob)},{float(x+55),float(y+18+bob)},{float(x+59),float(y+43)},{float(x+47),float(y+55)},{float(x+17),float(y+55)},{float(x+3),float(y+40)}},ink);
        c.rect(x+10,y+19+bob,42,29,0x6d846a); c.rect(x+12,y+20+bob,38,5,0xb4bf88);
        c.rect(x+13,y+27+bob,8,18,0x95a77c); c.rect(x+46,y+27+bob,6,18,0x425f50);
        c.rect(x+23,y+29+bob,22,19,0x344f49); c.circle(x+34,y+37+bob,9,0x92703d); c.circle(x+34,y+36+bob,6,0xf8c66d); star(c,x+34,y+36+bob,0xfff3b8,4);
        c.rect(x-8,y+19+bob,21,20,ink); c.rect(x+53,y+19+bob,20,20,ink);
        c.rect(x-6,y+20+bob,17,15,0x7d9270); c.rect(x+54,y+20+bob,17,15,0x6c8365);
        c.rect(x-8,y+17+bob,22,5,0x75a75c); c.rect(x+52,y+17+bob,23,5,0x639550);
        c.rect(x-9,y+35+bob,15,21,0x4b6d58); c.rect(x+60,y+35+bob,15,21,0x4b6d58);
        c.rect(x-8,y+51+bob,16,9,0x8b9c76); c.rect(x+59,y+51+bob,16,9,0x8b9c76);
        c.rect(x+18,y-1+bob,28,23,ink); c.rect(x+20,y+bob,24,19,0x8a9d78); c.rect(x+21,y+1+bob,21,4,0xc3cc95);
        c.rect(x+21,y+7+bob,21,6,0x284c42); c.rect(x+24,y+8+bob,5,3,gold); c.rect(x+35,y+8+bob,5,3,gold);
        c.rect(x+27,y+15+bob,9,2,0x405e49);
        c.rect(x+15,y-5+bob,10,7,0x57905c); c.rect(x+29,y-7+bob,15,9,0x75a765);
        c.line(x+28,y+21+bob,x+23,y+26+bob,0x435f4a); c.line(x+40,y+20+bob,x+44,y+27+bob,0x435f4a);
    }
    if(e.kind!=Kind::Boss&&e.hp<e.maxHP) {
        c.rect(x,y-9,25,3,ink); c.rect(x,y-9,std::max(0,25*e.hp/e.maxHP),2,0xf3bb75);
    }
}
void gate(Canvas& c,int x,float t) {
    c.rect(x-10,270,7,40,0x4b6d60); c.rect(x+13,270,7,40,0x4b6d60);
    c.rect(x-12,268,11,5,0x8fa87c); c.rect(x+11,268,11,5,0x8fa87c);
    c.poly({{float(x-7),271},{float(x),256},{float(x+10),256},{float(x+17),271}},0x5b8268);
    c.rect(x-2,272,15,38,0x183d3e);
    for(int i=0;i<5;++i) { int y=273+(int(t*30)+i*8)%35; c.rect(x+2,y,6,3,0x65bdb0); }
    star(c,x+5,262,gold,3);
}
void hud(const Game& g,Canvas& c) {
    panel(c,12,10,616,38);
    star(c,28,28,gold,6); c.text(42,19,"AMBER",cream); c.text(42,33,"ARCADE",0x91c6ae);
    for(int i=0;i<g.player.maxHP;++i) {
        int x=101+(i%5)*13,y=22+(i/5)*10; Color col=i<g.player.hp?0xec9b82:0x3d5b51;
        c.rect(x,y,3,3,col); c.rect(x+4,y,3,3,col); c.poly({{float(x),float(y+2)},{float(x+7),float(y+2)},{float(x+3),float(y+7)}},col);
    }
    c.text(177,19,"PONTOS",0x90bba5); c.text(177,33,number(g.score,7),cream);
    c.text(266,19,"ONDA",0x90bba5); c.text(266,33,number(g.wave,2)+" / 06",gold);
    c.text(352,19,"COMBO X"+number(g.multiplier()),gold); c.rect(352,36,68,2,0x36584f); c.rect(352,36,int(68*g.comboTimer/3.2f),2,gold);
    c.text(445,19,"PULSO "+number(g.pulses),0x90bba5); c.text(445,33,"K  ATIVAR",cream);
    c.text(527,19,"P PAUSA",cream); c.text(527,33,g.muted?"M MUDO":"M SOM",0x90bba5);
    panel(c,12,328,616,22);
    c.text(22,336,"A D MOVER   SPACE PULAR   J ATIRAR   SHIFT ESQUIVAR",cream);
    c.text(516,336,number(int(g.elapsed)/60,2)+":"+number(int(g.elapsed)%60,2),gold);
    int ready=int(56*(1-std::clamp(g.player.dashCooldown/1.3f,0.f,1.f)));
    c.rect(567,336,49,5,0x36594f); c.rect(567,336,ready*49/56,5,0x8dccb4);
    bool boss=false;
    for(const auto& e:g.enemies) if(e.kind==Kind::Boss&&e.alive) {
        boss=true; panel(c,177,54,286,21); c.text(187,60,"GUARDIAO",gold);
        c.rect(246,62,205,5,0x36584f); c.rect(246,62,205*e.hp/e.maxHP,5,0xdfad66);
    }
    if(!boss) {
        c.text(17,59,"AMEACAS "+number(g.aliveEnemies()+g.pending,2),0xc8d4b0);
        c.text(491,59,"ARMA NIVEL "+number(g.player.power-1),0xc8d4b0);
    }
    if(g.banner>0&&g.mode==Mode::Playing) {
        panel(c,203,87,234,43); c.centered(98,g.wave==6?"ULTIMA ONDA":"ONDA "+number(g.wave),gold,2);
        c.centered(119,g.wave==6?"O GUARDIAO DESPERTOU":"SEGURE J PARA ATIRAR",cream);
    }
}
}
void Canvas::clear(Color c) { std::fill(pixels.begin(),pixels.end(),c); }
void Canvas::pixel(int x,int y,Color c) { if(x>=0&&x<W&&y>=0&&y<H) pixels[y*W+x]=c; }
void Canvas::blend(int x,int y,Color c,int a) { if(x>=0&&x<W&&y>=0&&y<H) pixels[y*W+x]=mix(pixels[y*W+x],c,float(a)/255); }
void Canvas::rect(int x,int y,int w,int h,Color c) {
    int x0=std::max(0,x),x1=std::min(W,x+w),y0=std::max(0,y),y1=std::min(H,y+h);
    if(x1<=x0) return;
    for(int yy=y0;yy<y1;++yy) std::fill(pixels.begin()+yy*W+x0,pixels.begin()+yy*W+x1,c);
}
void Canvas::line(int x0,int y0,int x1,int y1,Color c) {
    int dx=std::abs(x1-x0),sx=x0<x1?1:-1,dy=-std::abs(y1-y0),sy=y0<y1?1:-1,e=dx+dy;
    for(;;) { pixel(x0,y0,c); if(x0==x1&&y0==y1) break; int e2=2*e; if(e2>=dy) { e+=dy; x0+=sx; } if(e2<=dx) { e+=dx; y0+=sy; } }
}
void Canvas::circle(int x,int y,int r,Color c) { ellipse(x,y,r,r,c); }
void Canvas::ellipse(int x,int y,int rx,int ry,Color c) {
    if(rx<=0||ry<=0) return;
    for(int yy=std::max(-ry,-y);yy<=ry&&y+yy<H;++yy) { float f=1-float(yy*yy)/float(ry*ry); int w=int(rx*std::sqrt(std::max(0.f,f))); rect(x-w,y+yy,w*2+1,1,c); }
}
void Canvas::poly(const std::vector<Vec>& p,Color col) {
    if(p.size()<3) return;
    float low=p[0].y,high=low; for(auto v:p) { low=std::min(low,v.y); high=std::max(high,v.y); }
    for(int y=std::max(0,int(low));y<=std::min(H-1,int(high));++y) {
        std::vector<float> nodes;
        for(size_t i=0,j=p.size()-1;i<p.size();j=i++) if((p[i].y<y&&p[j].y>=y)||(p[j].y<y&&p[i].y>=y)) nodes.push_back(p[i].x+(y-p[i].y)/(p[j].y-p[i].y)*(p[j].x-p[i].x));
        std::sort(nodes.begin(),nodes.end()); for(size_t i=0;i+1<nodes.size();i+=2) rect(int(nodes[i]),y,int(nodes[i+1]-nodes[i])+1,1,col);
    }
}
void Canvas::text(int x,int y,const std::string& s,Color c,int scale) {
    static const std::string letters="ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789:./-+!?%";
    static const std::array<std::array<int,7>,44> glyphs={{
        {{14,17,17,31,17,17,17}},{{30,17,17,30,17,17,30}},{{14,17,16,16,16,17,14}},{{30,17,17,17,17,17,30}},
        {{31,16,16,30,16,16,31}},{{31,16,16,30,16,16,16}},{{14,17,16,23,17,17,15}},{{17,17,17,31,17,17,17}},
        {{31,4,4,4,4,4,31}},{{7,2,2,2,18,18,12}},{{17,18,20,24,20,18,17}},{{16,16,16,16,16,16,31}},
        {{17,27,21,21,17,17,17}},{{17,25,21,19,17,17,17}},{{14,17,17,17,17,17,14}},{{30,17,17,30,16,16,16}},
        {{14,17,17,17,21,18,13}},{{30,17,17,30,20,18,17}},{{15,16,16,14,1,1,30}},{{31,4,4,4,4,4,4}},
        {{17,17,17,17,17,17,14}},{{17,17,17,17,17,10,4}},{{17,17,17,21,21,27,17}},{{17,17,10,4,10,17,17}},
        {{17,17,10,4,4,4,4}},{{31,1,2,4,8,16,31}},{{14,17,19,21,25,17,14}},{{4,12,4,4,4,4,14}},
        {{14,17,1,2,4,8,31}},{{30,1,1,14,1,1,30}},{{2,6,10,18,31,2,2}},{{31,16,16,30,1,1,30}},
        {{14,16,16,30,17,17,14}},{{31,1,2,4,8,8,8}},{{14,17,17,14,17,17,14}},{{14,17,17,15,1,1,14}},
        {{0,4,4,0,4,4,0}},{{0,0,0,0,0,6,6}},{{1,2,2,4,8,8,16}},{{0,0,0,31,0,0,0}},
        {{0,4,4,31,4,4,0}},{{4,4,4,4,4,0,4}},{{14,17,1,2,4,0,4}},{{25,25,2,4,8,19,19}}
    }};
    for(char ch:s) {
        size_t k=letters.find(char(std::toupper(static_cast<unsigned char>(ch))));
        if(k<glyphs.size()) for(int row=0;row<7;++row) for(int col=0;col<5;++col) if(glyphs[k][row]&(1<<(4-col))) rect(x+col*scale,y+row*scale,scale,scale,c);
        x+=6*scale;
    }
}
void Canvas::savePPM(const std::string& path) const {
    std::ofstream out(path,std::ios::binary); if(!out) throw std::runtime_error("Cannot create screenshot: "+path);
    out<<"P6\n"<<W<<" "<<H<<"\n255\n";
    for(Color p:pixels) { const char bytes[]={char(p>>16),char(p>>8),char(p)}; out.write(bytes,3); }
    if(!out) throw std::runtime_error("Failed writing screenshot");
}

void render(const Game& g,Canvas& c) {
    background(g,c);
    int shift=g.shake>0?int(std::sin(g.time*135)*2):0;
    for(auto s:g.solids) { s.x+=shift; terrain(c,s,0); }
    gate(c,27,g.time); gate(c,605,-g.time);
    // Amber crystals and flowers on the arena floor.
    for(int i=0;i<16;++i) {
        int x=19+i*40;
        c.line(x,309,x-2,303,0x7ca265); c.line(x,309,x+4,304,0x9ec476);
        if(i%3==0) { c.rect(x+5,299,4,4,0xf5c782); c.rect(x+4,300,6,2,0xd9a45a); }
    }
    for(const auto& drop:g.pickups) {
        int x=int(drop.p.x),y=int(drop.p.y+std::sin(g.time*5+drop.p.x)*2);
        if(drop.kind==Drop::Gem) { c.poly({{float(x),float(y-6)},{float(x+5),float(y)},{float(x),float(y+6)},{float(x-5),float(y)}},gold); c.line(x-1,y-3,x-1,y+2,cream); }
        else if(drop.kind==Drop::Health) { c.rect(x-6,y-6,12,12,ink); c.rect(x-4,y-4,8,8,0x507b62); c.rect(x-1,y-3,2,6,0xffb7a0); c.rect(x-3,y-1,6,2,0xffb7a0); }
        else { c.circle(x,y,7,ink); c.circle(x,y,5,gold); star(c,x,y,cream,3); }
    }
    for(const auto& e:g.enemies) if(e.alive) enemyArt(c,e,g.time);
    if(g.player.grounded) c.ellipse(int(g.player.pos.x)+9,int(g.player.pos.y)+30,12,2,0x254337);
    hero(c,int(g.player.pos.x)-1,int(g.player.pos.y)-2,g);
    for(const auto& b:g.bullets) if(b.alive) {
        int x=int(b.p.x),y=int(b.p.y); Color col=b.hostile?0xf3ab72:0x8fedd2;
        c.line(x-int(b.v.x*.024f),y-int(b.v.y*.024f),x,y,b.hostile?0x9f6549:0x3e8d81);
        c.circle(x,y,b.hostile?4:3,ink); c.circle(x,y,b.hostile?3:2,col); c.pixel(x-1,y-1,cream);
    }
    if(g.pulseRing>0) {
        float radius=170*(1-g.pulseRing/.65f);
        for(int i=0;i<180;++i) { float a=i*.0349066f; int x=int(g.player.center().x+std::cos(a)*radius),y=int(g.player.center().y+std::sin(a)*radius); c.rect(x,y,2,2,gold); }
    }
    for(const auto& fx:g.particles) c.rect(int(fx.p.x),int(fx.p.y),fx.size,fx.size,fx.color);
    for(int i=0;i<18;++i) { int x=(hash(i+890)%640+int(g.time*5))%640,y=95+hash(i+330)%200+int(std::sin(g.time+i)*4); if(i%3==0) star(c,x,y,0xb5be7c,1); else c.pixel(x,y,0xb5c391); }
    hud(g,c);
    if(g.mode==Mode::Title) {
        for(int y=66;y<318;++y) for(int x=23;x<421;++x) c.blend(x,y,0x102c34,205);
        c.text(41,84,"SOBREVIVA. EVOLUA. BATA SEU RECORDE.",0xa1c0a7);
        c.text(40,109,"AMBER",ink,6); c.text(38,106,"AMBER",cream,6);
        c.text(40,157,"ARCADE",ink,6); c.text(38,154,"ARCADE",gold,6);
        c.text(42,210,"A ARENA DO BOSQUE",0x98c8af,2);
        c.text(42,239,"6 ONDAS   3 MELHORIAS   1 GUARDIAO",cream);
        panel(c,40,263,344,30); c.text(52,274,"ENTER / SPACE / CLIQUE  INICIAR",gold);
        c.text(43,304,"RECORDE "+number(g.best,7)+"    F11 TELA CHEIA",0xa1c0a7);
    } else if(g.mode==Mode::Upgrade) {
        for(int y=0;y<H;++y) for(int x=0;x<W;++x) c.blend(x,y,0x0b252d,190);
        c.centered(61,"ONDA CONCLUIDA",gold,3);
        c.centered(94,"ESCOLHA UMA MELHORIA PARA A PROXIMA ONDA",cream);
        const char* titles[]={"PODER","VELOCIDADE","VITALIDADE"};
        const char* details[]={"DANO +1","TIROS +28%","VIDA MAXIMA +1"};
        const char* extra[]={"CRISTAIS MAIS FORTES","MAIS TIROS POR SEGUNDO","CURA COMPLETA"};
        for(int i=0;i<3;++i) {
            int x=35+i*195; panel(c,x,122,181,146);
            c.circle(x+90,153,15,0x345d52); star(c,x+90,153,i==1?0x8be4c8:gold,8);
            c.text(x+14,180,number(i+1)+" "+titles[i],gold,2);
            c.text(x+14,210,details[i],cream); c.text(x+14,230,extra[i],0x9ac1a8);
        }
        c.centered(291,"PRESSIONE 1 / 2 / 3 PARA ESCOLHER",cream);
        c.centered(313,"ENTER ESCOLHE PODER    R REINICIA",0x9fc1a8);
    } else if(g.mode!=Mode::Playing) {
        for(int y=0;y<H;++y) for(int x=0;x<W;++x) c.blend(x,y,0x0b252d,180);
        panel(c,115,91,410,186);
        c.centered(112,g.mode==Mode::Paused?"PAUSADO":g.mode==Mode::Won?"BOSQUE PROTEGIDO":"FIM DE JOGO",gold,2);
        c.centered(148,g.mode==Mode::Won?"AS SEIS ONDAS FORAM VENCIDAS.":g.mode==Mode::Paused?"A ARENA ESPERA POR VOCE.":"O PROXIMO RECORDE COMECA AGORA.",cream);
        c.centered(173,"PONTOS "+number(g.score,7)+"  RECORDE "+number(g.best,7),cream);
        c.centered(194,"ONDA "+number(g.wave)+" / 6   INIMIGOS "+number(g.kills),0x98c3a7);
        c.centered(222,"TEMPO "+number(int(g.elapsed)/60,2)+":"+number(int(g.elapsed)%60,2),gold);
        c.centered(250,g.mode==Mode::Paused?"P / ESC VOLTAR     R REINICIAR":"ENTER JOGAR NOVAMENTE",cream);
    }
}
} // namespace arcade
