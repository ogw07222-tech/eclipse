#include "../src/core.hpp"
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
        for(int combo=1;combo<=3;++combo){e.x=combo==3?230:200;e.speed=0;g.step(1.f/60,a);check(e.health==health&&g.player.combo==combo,"Hit skipped wind-up");wait(g,combo==3?14:13);health-=combo==1?12:combo==2?15:23;check(e.health==health,"Contact damage");if(combo==3)check(e.bleed>4,"Finisher bleed");wait(g,20);}
    });
    test("Wind-up cancellation cannot leave a ghost sword hit",[]{for(int mode=0;mode<4;++mode){auto g=arena();g.enemies[0].x=200;Actions a;a.attack=true;g.step(1.f/60,a);a={};
        if(mode==0)a.dash=true;else if(mode==1)a.swap=true;else if(mode==2)g.damagePlayer(1);else a.restart=true;
        g.step(1.f/60,a);check(g.player.meleeLeft==0,"Pending slash survived cancellation");int hp=g.enemies[0].health;wait(g,16);check(g.enemies[0].health==hp,"Cancelled slash dealt delayed damage");
    }});
    test("Contact resolves current position once and respects gear speed",[]{for(int fast=0;fast<2;++fast){auto g=arena();if(fast)g.player.gear.selected[0]=2;Actions a;a.attack=true;g.step(1.f/60,a);g.enemies[0].x=200;int elapsed=1;while(g.player.meleeLeft>0&&elapsed<20){wait(g,1);++elapsed;}
        check(elapsed==(fast?5:6),"Wind-up duration does not follow clip/gear");wait(g,2);check(g.enemies[0].health==(fast?45:43),"Moved target not hit");int hp=g.enemies[0].health;g.enemies[0].x=200;wait(g,14);check(g.enemies[0].health==hp,"Sword hit twice");
    }});
    test("Target leaving reach during wind-up is not hit",[]{auto g=arena();g.enemies[0].x=200;Actions a;a.attack=true;g.step(1.f/60,a);g.enemies[0].x=500;wait(g,12);check(g.enemies[0].health==55&&g.player.meleeLeft==0,"Slash used stale target position");});
    test("Combo window timeout",[]{auto g=arena();Actions a;a.attack=true;g.step(1.f/60,a);wait(g,55);g.step(1.f/60,a);check(g.player.combo==1,"Reset combo");});
    test("Bleed fire reaction consumes once",[]{auto g=arena();auto& e=g.enemies[0];g.hit(e,1,false,true);g.hit(e,11,true);check(e.health==21&&e.bleed==0&&e.burn>0&&e.reactionFlash>0,"Reaction +22");g.hit(e,1,true);check(e.health==20,"No repeat reaction");});
    test("Bleed and burn ticks",[]{auto g=arena();g.hit(g.enemies[0],1,false,true);wait(g,46);check(g.enemies[0].health==51,"Bleed tick");g.hit(g.enemies[1],1,true);wait(g,37);check(g.enemies[1].health==82,"Burn tick");});
    test("Both swap entry attacks cooldown",[]{auto g=arena();Actions a;a.swap=true;g.step(1.f/60,a);check(g.player.form==1&&g.projectiles.size()==1&&g.projectiles[0].damage==14,"Mage entry");g.step(1.f/60,a);check(g.player.form==1&&g.projectiles.size()==1,"Swap cooldown");wait(g,61);g.enemies[0].x=200;g.step(1.f/60,a);check(g.player.form==0&&g.enemies[0].health==55,"Sword entry wind-up");wait(g,15);check(g.enemies[0].health==42,"Sword entry");});
    test("Mage empowered projectile collision",[]{auto g=arena();g.player.form=1;Actions a;a.skill=true;g.step(1.f/60,a);check(g.projectiles.size()==1&&g.projectiles[0].empowered,"Skill");near(g.player.skillCooldown,4.1f);g.enemies[0].x=230;wait(g,12);check(g.enemies[0].health==30&&g.enemies[0].burn>0,"Collision and burn");});
    test("Nine item stats health adjustment",[]{Player p;p.equip(0);p.equip(1);check(p.gear.attack()==6&&p.gear.defense()==4&&p.maxHealth()==130&&p.health==130,"Armor weapon");near(p.gear.speed(),.792f);near(p.gear.range(),1.16f);p.equip(2);check(p.gear.attack()==9,"Flame charm");p.equip(1);check(p.health==110&&p.maxHealth()==110,"Health adjustment");p.equip(2);check(p.gear.defense()==3&&p.maxHealth()==125,"Guardian charm");});
    test("Damage immunity defeat freeze",[]{auto g=arena();g.damagePlayer(9);g.damagePlayer(9);check(g.player.health==91,"Damage immunity");wait(g,45);g.damagePlayer(999);check(g.ended&&!g.won&&g.player.health==0,"Defeat");float x=g.player.x;Actions a;a.right=true;g.step(1.f/60,a);near(g.player.x,x);});
    test("Stage clear",[]{auto g=arena();for(auto& e:g.enemies)g.hit(e,1000,false);g.step(1.f/60,{});check(g.ended&&g.won&&g.foes()==0,"Clear");});
    test("Restart clears gear projectiles effects",[]{auto g=arena();g.player.form=1;g.player.equip(0);Actions a;a.attack=true;g.step(1.f/60,a);g.hit(g.enemies[0],1,true);a={};a.restart=true;g.step(1.f/60,a);check(g.foes()==4&&g.projectiles.empty()&&g.popups.empty()&&!g.ended&&g.player.form==0&&g.player.gear.selected[0]==0,"Restart state");});
    test("Rear and vertical melee excluded",[]{auto g=arena();g.enemies[0].x=80;Actions a;a.attack=true;g.step(1.f/60,a);check(g.enemies[0].health==55,"Rear hit");wait(g,18);g.player.y=400;g.player.grounded=false;g.enemies[0].x=200;a={};a.skill=true;g.step(1.f/60,a);check(g.enemies[0].health==55,"Vertical hit");});
    test("Multitouch independent keyboard ownership",[]{Input in;in.pointer(1,60,650,true);in.pointer(2,1160,590,true);auto a=in.sample();check(a.left&&a.attack,"Move plus attack");check(!in.sample().attack,"Held attack edge");in.pointer(2,0,0,false);check(in.sample().left,"Attack up retains movement");Actions k;k.right=true;in.keyboard(k);in.pointer(1,0,0,false);check(in.sample().right,"Keyboard retained");in.reset();check(!in.sample().right,"Reset");});
    test("Fast tap buffered between simulation steps",[]{Input in;in.pointer(1,1160,590,true);in.pointer(1,0,0,false);check(in.sample().attack,"Fast touch lost");check(!in.sample().attack,"Single consume");});
    test("Joystick upward jump and focus reset",[]{Input in;in.pointer(1,99,610,true);check(in.sample().jump,"Up jump");check(in.sample().jump,"Held upward jump lost");in.reset();check(!in.sample().jump,"Focus clear");});
    test("Inventory equip and retry touch targets",[]{Input in;in.pointer(1,800,40,true);check(in.sample().inventory,"Inventory");in.reset();in.pointer(2,90,430,true,false,true);check(in.sample().weapon,"Weapon");in.reset();in.pointer(3,600,440,true,true,false);check(in.sample().restart,"Retry");});
    test("Animation attack retrigger and death bounds",[]{Player p;Animation a;++p.attackSerial;p.combo=3;a.step(p,1.f/60);check(a.frame>=144&&a.frame<154,"Third attack atlas");p.health=0;for(int i=0;i<100;++i)a.step(p,1.f/60);check(a.frame==187,"Death last frame");});
    test("Every combo reaches its final frame and recovers",[]{for(int combo=1;combo<=3;++combo){Player p;p.combo=combo;++p.attackSerial;Animation a;int largest=0;for(int i=0;i<70;++i){a.step(p,1.f/60);if(a.state==3+combo)largest=std::max(largest,a.frame);}const auto clip=motionClips[3+combo];check(largest==clip.start+clip.count-1&&a.state==0,"Truncated clip");}});
    test("Run cycle follows actual world travel without wall skating",[]{Player p;p.vx=280;Animation a;a.step(p,1.f/60);int first=a.frame;for(int i=0;i<16;++i){p.x+=7.5f;a.step(p,1.f/60);}check(a.frame==first,"Stride distance mismatch");for(int i=0;i<30;++i)a.step(p,1.f/60);check(a.frame==first,"Stationary feet animated");});
    test("Jump apex fall and landing use distinct clips",[]{Player p;Animation a;p.grounded=false;p.vy=-650;a.step(p,1.f/60);check(a.state==2&&a.frame==48,"Takeoff");p.vy=350;a.step(p,1.f/60);check(a.state==3&&a.frame>=64,"Fall");p.grounded=true;p.vy=0;a.step(p,1.f/60);check(a.state==10,"Landing missing");for(int i=0;i<12;++i)a.step(p,1.f/60);check(a.state==0,"Landing stuck");});
    test("Hit-stop holds the contact pose then resumes",[]{Player p;p.combo=2;++p.attackSerial;p.hitStop=.075f;Animation a;a.step(p,1.f/60);const int contact=a.frame;check(contact>=133,"Hit froze before contact");a.step(p,1.f/60);check(a.frame==contact,"Hit-stop pose advanced");p.hitStop=0;a.step(p,.05f);check(a.frame>contact,"Contact did not resume");});
    test("Held joystick bounces at contact without a grounded delay or air jumps",[]{auto g=arena();Input in;in.pointer(1,99,610,true);
        int bounces=0;float lastVy=0;
        for(int i=0;i<210;++i){float oldY=g.player.y;g.step(1.f/60,in.sample());
            if(lastVy>0&&g.player.vy<0){++bounces;check(g.player.y==floorY&&!g.player.grounded,"Delayed landing jump");}
            if(g.player.vy<lastVy-100)check(i==0||oldY>floorY-15,"Air jump retrigger");
            lastVy=g.player.vy;
        }
        check(bounces>=3,"Did not keep jumping");in.pointer(1,0,0,false);wait(g,80);check(g.player.grounded,"Release failed to stop repeat");
    });
    test("Buffered jump at landing and multiple-pointer release",[]{auto g=arena();g.player.y=floorY-1;g.player.vy=200;g.player.grounded=false;
        Actions a;a.jump=true;g.step(1.f/60,a);check(!g.player.grounded&&g.player.vy<0&&g.player.y==floorY,"Landing buffer");
        Input in;in.pointer(1,99,610,true);in.pointer(2,99,610,true);in.sample();in.pointer(1,0,0,false);check(in.sample().jump,"Other jump owner lost");
    });
    test("Player hit flash expires while dash immunity blocks it",[]{auto g=arena();g.damagePlayer(9);check(g.player.hitFlash>0,"No player flash");wait(g,12);check(g.player.hitFlash==0&&g.player.invulnerable>0,"Flash coupled to immunity");
        g.restart();Actions a;a.dash=true;g.step(1.f/60,a);g.damagePlayer(9);check(g.player.hitFlash==0,"Immune hit flashed");
    });
    test("Melee mirrors and cannot hit past the visible blade",[]{for(int facing:{-1,1})for(int combo=1;combo<=3;++combo){auto g=arena();g.player.x=600;g.player.facing=facing;g.player.combo=combo-1;g.player.comboWindow=1;
        auto& e=g.enemies[0];e.speed=0;e.x=600+facing*60;Actions a;a.attack=true;g.step(1.f/60,a);wait(g,35);check(e.health<55,"Mirrored close blade missed");
        auto far=arena();far.player.x=600;far.player.facing=facing;far.player.combo=combo-1;far.player.comboWindow=1;far.enemies[0].speed=0;far.enemies[0].x=600+facing*180;
        far.step(1.f/60,a);wait(far,35);check(far.enemies[0].health==55,"Invisible long range damage");
    }});
    test("Later slash contact hits each enemy only once",[]{auto g=arena();auto& e=g.enemies[0];e.speed=0;e.x=500;Actions a;a.attack=true;g.step(1.f/60,a);wait(g,5);
        e.x=200;wait(g,12);check(e.health==43,"Active slash did not accept late entry");e.x=200;wait(g,12);check(e.health==43,"Duplicate swing hit");
    });
    test("Settings modal freezes game and preserves preferences on restart",[]{auto g=arena();Actions a;a.settings=true;g.step(1.f/60,a);check(g.settings,"Open settings");float time=g.clock,x=g.player.x;
        a={};a.right=a.attack=a.skill=a.inventory=a.weapon=true;a.shake=a.damageNumbers=a.afterimages=true;g.step(1.f/60,a);
        near(g.clock,time);near(g.player.x,x);check(g.player.gear.selected[0]==0&&g.projectiles.empty()&&!g.inventory,"Modal leaked gameplay");
        check(!g.screenShake&&!g.showDamage&&!g.showAfterimages,"Settings toggles");a={};a.restart=true;g.step(1.f/60,a);
        check(!g.settings&&g.player.health==100&&!g.screenShake&&!g.showDamage&&!g.showAfterimages,"Restart reset preferences");
    });
    test("Settings touch consumes only modal controls",[]{Input in;in.pointer(1,1230,40,true);check(in.sample().settings,"Gear hit target");check(!in.sample().settings,"Held gear retriggers");in.reset();
        in.pointer(1,1160,590,true,false,false,true);check(!in.sample().attack,"Attack behind modal");in.reset();
        in.pointer(1,780,270,true,false,false,true);check(in.sample().shake,"Shake row");in.reset();in.pointer(1,780,330,true,false,false,true);check(in.sample().damageNumbers,"Numbers row");
        in.reset();in.pointer(1,780,390,true,false,false,true);check(in.sample().afterimages,"Afterimage row");in.reset();in.pointer(1,600,460,true,false,false,true);check(in.sample().settings,"Resume target");
    });
    std::cout<<total-failed<<'/'<<total<<" passed\n";return failed?1:0;
}
