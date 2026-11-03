#include "../src/core.hpp"
#include "../src/forest_generator.hpp"
#include <cstdlib>
#include <iostream>
#include <stdexcept>
using namespace eclipse;
void check(bool b,const char* message){if(!b)throw std::runtime_error(message);}
void near(float a,float b){check(std::abs(a-b)<.025f,"Numeric value differs");}
void wait(Combat& g,int frames){for(int i=0;i<frames;++i)g.step(1.f/60,{});}
Combat arena(){Combat g;for(auto& e:g.enemies){e.x=1500;e.attackTimer=1000;}return g;}
int main(){int failed=0,total=0;
    auto test=[&](const char* name,auto fn){++total;try{fn();std::cout<<"PASS "<<name<<'\n';}catch(const std::exception& e){++failed;std::cerr<<"FAIL "<<name<<": "<<e.what()<<'\n';}};
    test("Initial four enemy wave",[]{Combat g;check(g.foes()==4&&g.player.health==100&&!g.ended,"Initial wave");check(g.enemies[0].health==55&&g.enemies[1].health==85,"Enemy types");near(g.enemies[3].x,1285);});
    test("Movement facing walls",[]{auto g=arena();Actions a;a.right=true;g.step(1.f/60,a);near(g.player.x,140+280.f/60);g.player.x=1549;g.step(1.f/60,a);near(g.player.x,1550);a={};a.left=true;g.player.x=26;g.step(1.f/60,a);near(g.player.x,25);check(g.player.facing== -1,"Facing");});
    test("Jump apex and landing",[]{auto g=arena();Actions a;a.jump=true;g.step(1.f/60,a);check(!g.player.grounded&&g.player.vy<0,"Jump");float highest=g.player.y;for(int i=0;i<90;++i){g.step(1.f/60,{});highest=std::min(highest,g.player.y);}check(floorY-highest>160&&floorY-highest<180,"Apex");check(g.player.grounded&&g.player.vy==0,"Landing");});
    test("Dash immunity and cooldown",[]{auto g=arena();Actions a;a.dash=true;g.step(1.f/60,a);near(g.player.vx,780);g.damagePlayer(15);check(g.player.health==100,"Dash immunity");wait(g,12);float cooldown=g.player.dashCooldown;g.step(1.f/60,a);check(g.player.dashLeft==0&&g.player.dashCooldown<cooldown,"Cooldown");wait(g,60);g.step(1.f/60,a);check(g.player.dashLeft>0,"Reactivation");});
    test("Three hit combo damage lands at the blade contact",[]{auto g=arena();auto& e=g.enemies[0];Actions a;a.attack=true;int health=55;
        for(int combo=1;combo<=3;++combo){e.x=200;g.step(1.f/60,a);check(e.health==health&&g.player.combo==combo,"Hit skipped wind-up");wait(g,combo==3?14:13);health-=combo==1?12:combo==2?15:23;check(e.health==health,"Contact damage");if(combo==3)check(e.bleed>4,"Finisher bleed");wait(g,20);}
    });
    test("Wind-up cancellation cannot leave a ghost sword hit",[]{for(int mode=0;mode<4;++mode){auto g=arena();g.enemies[0].x=200;Actions a;a.attack=true;g.step(1.f/60,a);a={};
        if(mode==0)a.dash=true;else if(mode==1)a.swap=true;else if(mode==2)g.damagePlayer(1);else a.restart=true;
        g.step(1.f/60,a);check(g.player.meleeLeft==0,"Pending slash survived cancellation");int hp=g.enemies[0].health;wait(g,16);check(g.enemies[0].health==hp,"Cancelled slash dealt delayed damage");
    }});
    test("Contact resolves current position once and respects gear speed",[]{for(int fast=0;fast<2;++fast){auto g=arena();if(fast)g.player.gear.selected[0]=2;Actions a;a.attack=true;g.step(1.f/60,a);g.enemies[0].x=200;int elapsed=1;while(g.player.meleeLeft>0&&elapsed<20){wait(g,1);++elapsed;}
        check(elapsed==(fast?5:6),"Wind-up duration does not follow clip/gear");check(g.enemies[0].health==(fast?45:43),"Moved target not hit");int hp=g.enemies[0].health;g.enemies[0].x=200;wait(g,14);check(g.enemies[0].health==hp,"Sword hit twice");
    }});
    test("Target leaving reach during wind-up is not hit",[]{auto g=arena();g.enemies[0].x=200;Actions a;a.attack=true;g.step(1.f/60,a);g.enemies[0].x=500;wait(g,12);check(g.enemies[0].health==55&&g.player.meleeLeft==0,"Slash used stale target position");});
    test("Combo window timeout",[]{auto g=arena();Actions a;a.attack=true;g.step(1.f/60,a);wait(g,55);g.step(1.f/60,a);check(g.player.combo==1,"Reset combo");});
    test("Bleed fire reaction consumes once",[]{auto g=arena();auto& e=g.enemies[0];g.hit(e,1,false,true);g.hit(e,11,true);check(e.health==21&&e.bleed==0&&e.burn>0&&e.reactionFlash>0,"Reaction +22");g.hit(e,1,true);check(e.health==20,"No repeat reaction");});
    test("Bleed and burn ticks",[]{auto g=arena();g.hit(g.enemies[0],1,false,true);wait(g,46);check(g.enemies[0].health==51,"Bleed tick");g.hit(g.enemies[1],1,true);wait(g,37);check(g.enemies[1].health==82,"Burn tick");});
    test("Both swap entry attacks cooldown",[]{auto g=arena();Actions a;a.swap=true;g.step(1.f/60,a);check(g.player.form==1&&g.projectiles.size()==1&&g.projectiles[0].damage==14,"Mage entry");g.step(1.f/60,a);check(g.player.form==1&&g.projectiles.size()==1,"Swap cooldown");wait(g,61);g.enemies[0].x=200;g.step(1.f/60,a);check(g.player.form==0&&g.enemies[0].health==42,"Sword entry");});
    test("Mage empowered projectile collision",[]{auto g=arena();g.player.form=1;Actions a;a.skill=true;g.step(1.f/60,a);check(g.projectiles.size()==1&&g.projectiles[0].empowered,"Skill");near(g.player.skillCooldown,4.1f);g.enemies[0].x=230;wait(g,12);check(g.enemies[0].health==30&&g.enemies[0].burn>0,"Collision and burn");});
    test("Nine item stats health adjustment",[]{Player p;p.equip(0);p.equip(1);check(p.gear.attack()==6&&p.gear.defense()==4&&p.maxHealth()==130&&p.health==130,"Armor weapon");near(p.gear.speed(),.792f);near(p.gear.range(),1.16f);p.equip(2);check(p.gear.attack()==9,"Flame charm");p.equip(1);check(p.health==110&&p.maxHealth()==110,"Health adjustment");p.equip(2);check(p.gear.defense()==3&&p.maxHealth()==125,"Guardian charm");});
    test("Damage immunity defeat freeze",[]{auto g=arena();g.damagePlayer(9);g.damagePlayer(9);check(g.player.health==91,"Damage immunity");wait(g,45);g.damagePlayer(999);check(g.ended&&!g.won&&g.player.health==0,"Defeat");float x=g.player.x;Actions a;a.right=true;g.step(1.f/60,a);near(g.player.x,x);});
    test("Stage clear",[]{auto g=arena();for(auto& e:g.enemies)g.hit(e,1000,false);g.step(1.f/60,{});check(g.ended&&g.won&&g.foes()==0,"Clear");});
    test("Restart clears gear projectiles effects",[]{auto g=arena();g.player.form=1;g.player.equip(0);Actions a;a.attack=true;g.step(1.f/60,a);g.hit(g.enemies[0],1,true);a={};a.restart=true;g.step(1.f/60,a);check(g.foes()==4&&g.projectiles.empty()&&g.popups.empty()&&!g.ended&&g.player.form==0&&g.player.gear.selected[0]==0,"Restart state");});
    test("Rear and vertical melee excluded",[]{auto g=arena();g.enemies[0].x=80;Actions a;a.attack=true;g.step(1.f/60,a);check(g.enemies[0].health==55,"Rear hit");wait(g,18);g.player.y=400;g.player.grounded=false;g.enemies[0].x=200;a={};a.skill=true;g.step(1.f/60,a);check(g.enemies[0].health==55,"Vertical hit");});
    test("Multitouch independent keyboard ownership",[]{Input in;in.pointer(1,60,650,true);in.pointer(2,1160,590,true);auto a=in.sample();check(a.left&&a.attack,"Move plus attack");check(!in.sample().attack,"Held attack edge");in.pointer(2,0,0,false);check(in.sample().left,"Attack up retains movement");Actions k;k.right=true;in.keyboard(k);in.pointer(1,0,0,false);check(in.sample().right,"Keyboard retained");in.reset();check(!in.sample().right,"Reset");});
    test("Fast tap buffered between simulation steps",[]{Input in;in.pointer(1,1160,590,true);in.pointer(1,0,0,false);check(in.sample().attack,"Fast touch lost");check(!in.sample().attack,"Single consume");});
    test("Joystick upward jump and focus reset",[]{Input in;in.pointer(1,99,610,true);check(in.sample().jump,"Up jump");check(!in.sample().jump,"Held jump repeated");in.reset();check(!in.sample().jump,"Focus clear");});
    test("Inventory equip and retry touch targets",[]{Input in;in.pointer(1,800,40,true);check(in.sample().inventory,"Inventory");in.reset();in.pointer(2,90,430,true,false,true);check(in.sample().weapon,"Weapon");in.reset();in.pointer(3,600,440,true,true,false);check(in.sample().restart,"Retry");});
    test("Animation attack retrigger and death bounds",[]{Player p;Animation a;++p.attackSerial;p.combo=3;a.step(p,1.f/60);check(a.frame>=144&&a.frame<154,"Third attack atlas");p.health=0;for(int i=0;i<100;++i)a.step(p,1.f/60);check(a.frame==187,"Death last frame");});
    test("Every combo reaches its final frame and recovers",[]{for(int combo=1;combo<=3;++combo){Player p;p.combo=combo;++p.attackSerial;Animation a;int largest=0;for(int i=0;i<70;++i){a.step(p,1.f/60);if(a.state==3+combo)largest=std::max(largest,a.frame);}const auto clip=motionClips[3+combo];check(largest==clip.start+clip.count-1&&a.state==0,"Truncated clip");}});
    test("Run cycle follows actual world travel without wall skating",[]{Player p;p.vx=280;Animation a;a.step(p,1.f/60);int first=a.frame;for(int i=0;i<16;++i){p.x+=7.5f;a.step(p,1.f/60);}check(a.frame==first,"Stride distance mismatch");for(int i=0;i<30;++i)a.step(p,1.f/60);check(a.frame==first,"Stationary feet animated");});
    test("Jump apex fall and landing use distinct clips",[]{Player p;Animation a;p.grounded=false;p.vy=-650;a.step(p,1.f/60);check(a.state==2&&a.frame==48,"Takeoff");p.vy=350;a.step(p,1.f/60);check(a.state==3&&a.frame>=64,"Fall");p.grounded=true;p.vy=0;a.step(p,1.f/60);check(a.state==10,"Landing missing");for(int i=0;i<12;++i)a.step(p,1.f/60);check(a.state==0,"Landing stuck");});
    test("Hit-stop holds the contact pose then resumes",[]{Player p;p.combo=2;++p.attackSerial;p.hitStop=.075f;Animation a;a.step(p,1.f/60);const int contact=a.frame;check(contact>=133,"Hit froze before contact");a.step(p,1.f/60);check(a.frame==contact,"Hit-stop pose advanced");p.hitStop=0;a.step(p,.05f);check(a.frame>contact,"Contact did not resume");});
    test("Forest seed replay and isolation",[]{
        ForestSettings s;s.seed=22;ForestGenerator a(s),b(s);
        check(a.sprites().size()==b.sprites().size(),"Unstable count");
        for(size_t i=0;i<a.sprites().size();++i) {
            const auto& x=a.sprites()[i];const auto& y=b.sprites()[i];
            check(x.kind==y.kind&&x.variant==y.variant&&x.x==y.x&&x.scale==y.scale&&x.flipX==y.flipX&&x.shade==y.shade&&x.widthFactor==y.widthFactor,"Unstable sprite");
        }
        s.seed=23;ForestGenerator c(s);
        bool different=c.sprites().size()!=a.sprites().size();
        for(size_t i=0;i<std::min(c.sprites().size(),a.sprites().size());++i)
            different|=c.sprites()[i].x!=a.sprites()[i].x;
        check(different,"Seed ignored");
    });
    test("Forest variation is seeded across every active species",[]{
        ForestSettings s;s.seed=20261103;s.density=1;ForestGenerator a(s),b(s);
        int treeFlipped=0,treeNormal=0,propFlipped=0,propNormal=0;
        bool widthChanged=false,hasShade=false;
        for(size_t i=0;i<a.sprites().size();++i){
            const auto& x=a.sprites()[i];const auto& y=b.sprites()[i];
            check(x.flipX==y.flipX&&x.shade==y.shade&&x.widthFactor==y.widthFactor,
                "Unstable forest transformation");
            check(x.shade<=2&&x.widthFactor>.90f&&x.widthFactor<1.10f,
                "Subtle variation escaped limits");
            widthChanged|=x.widthFactor!=1.f;hasShade|=x.shade!=2;
            if(x.kind==ForestKind::LargeTree||x.kind==ForestKind::MidTree){
                if(x.flipX)++treeFlipped;else ++treeNormal;
            }else{
                if(x.flipX)++propFlipped;else ++propNormal;
            }
        }
        check(treeFlipped>0&&treeNormal>0,"Tree flip diversity absent");
        check(propFlipped>0&&propNormal>0,"Other props still repeat identically");
        check(widthChanged&&hasShade,"No subtle shape/tone differences");
    });
    test("Forest bounded density and sorted layers",[]{
        ForestSettings s;s.density=0;ForestGenerator empty(s);
        check(empty.sprites().empty(),"Zero density spawns sprites");
        s.density=1;ForestGenerator dense(s);
        check(dense.sprites().size()>20,"Density ineffective");
        for(size_t i=1;i<dense.sprites().size();++i)
            check(dense.sprites()[i].layer>=dense.sprites()[i-1].layer,"Layer order");
    });
    test("Forest no global grass or tree animation",[]{
        ForestGenerator g;ForestSprite trunk{ForestKind::LargeTree,0,1,100,585,1,0};
        ForestSprite grass{ForestKind::Grass,0,4,100,585,1,0};
        ForestSprite light{ForestKind::Effect,0,5,100,520,1,0};
        check(g.microMotion(trunk,0)==g.microMotion(trunk,9),"Tree sways");
        check(g.microMotion(grass,0)==g.microMotion(grass,9),"Grass sways");
        check(std::abs(g.microMotion(light,3))<2,"Excessive ambient motion");
    });
    test("Forest only two stable tree variants and fewer props",[]{
        ForestGenerator g;
        int main=0,secondary=0,bushes=0,details=0,grass=0;
        for(const auto& s:g.sprites()){
            if(s.kind==ForestKind::LargeTree){++main;check(s.variant==0,"Main variant");check(s.scale>2.8f,"Main too small");}
            if(s.kind==ForestKind::MidTree){++secondary;check(s.variant==0,"Secondary variant");check(s.scale>2.2f,"Secondary too small");}
            if(s.kind==ForestKind::Bush){++bushes;check(s.variant<2,"Too many bush types");}
            if(s.kind==ForestKind::Detail){++details;check(s.variant<2,"Too many detail types");}
            if(s.kind==ForestKind::Grass){++grass;check(s.variant<2,"Too many grass types");}
        }
        check(main>secondary,"Main trees should dominate");
        check(bushes+details+grass<40,"Too many foreground props");
    });
    test("Forest param bounds and camera parallax",[]{
        ForestSettings s;s.density=3;s.size=9;s.motion=9;
        ForestGenerator g(s);
        check(g.settings().density==1&&g.settings().size==1.8f&&g.settings().motion==.2f,"Clamps missing");
        ForestSprite f{ForestKind::Far,0,0,600,560,1,0};
        ForestSprite n{ForestKind::Grass,0,4,600,585,1,0};
        check(g.drawX(f,740)>g.drawX(n,740),"Parallax reversed");
        check(g.drawX(f,640)==600,"Default camera offset wrong");
    });
    std::cout<<total-failed<<'/'<<total<<" passed\n";return failed?1:0;
}
