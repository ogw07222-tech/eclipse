#include "../src/core.hpp"
#include "../src/forest_generator.hpp"
#include "../src/forest_layers.hpp"
#include "../src/story_director.hpp"
#include "../src/memory_menu.hpp"
#include <cstdio>
#include <set>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
using namespace eclipse;
void check(bool b,const char* message){if(!b)throw std::runtime_error(message);}
void near(float a,float b){check(std::abs(a-b)<.025f,"Numeric value differs");}
void wait(Combat& g,int frames){for(int i=0;i<frames;++i)g.step(1.f/60,{});}
Combat arena(){Combat g;for(auto& e:g.enemies){e.x=1500;e.attackTimer=1000;}return g;}
void tickStory(StoryDirector& d,Combat& g,Actions a,int frames){
    for(int i=0;i<frames;++i){Actions in=a;d.preStep(1.f/60,g,in);g.step(1.f/60,in);d.postStep(1.f/60,g);}
}
Combat kingArena(int kind){
    Combat g;g.scriptedSpawns=true;g.endOnClear=false;g.suppressGameOver=true;g.playerHealthFloor=1;
    g.configureCastleArea(5);g.spawn(kind,1000,0);g.player.x=700;g.player.invulnerable=0;
    for(auto& e:g.enemies)e.attackTimer=1000; // only forced moves run
    return g;
}
Enemy* kingOf(Combat& g){for(auto& e:g.enemies)if(e.kind==DemonKingArmored||e.kind==DemonKingTrue)return &e;return nullptr;}
void clearFoes(Combat& g){for(auto& e:g.enemies)e.health=0;}
int main(){int failed=0,total=0;
    auto test=[&](const char* name,auto fn){++total;try{fn();std::cout<<"PASS "<<name<<'\n';}catch(const std::exception& e){++failed;std::cerr<<"FAIL "<<name<<": "<<e.what()<<'\n';}};
    test("Default touch HUD keeps combat area clear without hitbox overlap",[]{
        const auto& buttons=ControlLayout::defaultButtons;
        for(int i=0;i<ControlLayout::Count;++i){
            check(buttons[i].size>=64,"Touch control is below the 64px target minimum");
            check(buttons[i].x>=0&&buttons[i].y>=0&&buttons[i].x+buttons[i].size<=1280&&
                buttons[i].y+buttons[i].size<=720,"Touch control leaves the logical screen");
            for(int j=i+1;j<ControlLayout::Count;++j){
                const auto& a=buttons[i];const auto& b=buttons[j];
                const bool overlap=a.x<b.x+b.size&&a.x+a.size>b.x&&
                    a.y<b.y+b.size&&a.y+a.size>b.y;
                check(!overlap,"Default touch hitboxes overlap");
            }
        }
        check(buttons[ControlLayout::Skill].y>=450&&buttons[ControlLayout::Ultimate].y>=500,
            "Skill cluster intrudes into the upper combat lane");
    });
    test("Settings vertical navigation and controls persistence",[]{
        ControlLayout ui;
        check(ui.pointer(1,ControlLayout::gearX+8,ControlLayout::gearY+8,true),"Gear should open menu");
        check(ui.editing,"Settings did not open");
        ui.pointer(1,75,235,true);
        check(ui.page==ControlLayout::Sound,"Sound category missing");
        ui.pointer(1,850,290,true);
        check(ui.bgm>=60,"BGM slider failed");
        ui.pointer(1,76,322,true);
        check(ui.page==ControlLayout::Controls,"Controls category missing");
        ui.pointer(1,490,312,true);
        check(ui.reposition,"Reposition should open");
        ui.pointer(1,1080,585,true);
        ui.pointer(1,1080,585,false);
        ui.pointer(2,1120,585,true);
        ui.pointer(3,1140,595,true);
        ui.pointer(3,1190,595,true);
        check(ui.buttons[ControlLayout::Attack].size>=116,"Pinch resize did not enlarge attack button");
        ui.pointer(3,1190,595,false);ui.pointer(2,1120,585,false);
        ui.pointer(1,1080,110,true);
        check(!ui.reposition,"Reposition did not close");
        ui.pointer(1,1145,130,true);
        check(!ui.editing,"Settings did not close");
    });
    test("Actors share the inset forest collision floor",[]{
        near(floorY,ForestLayerScene::floorY);
        near(floorY,ForestLayerScene::surfaceY+12);
        Combat g;near(g.player.y,floorY);
        for(const auto& e:g.enemies)near(e.y,floorY);
        g.player.y=floorY-1;g.player.vy=100;g.player.grounded=false;
        g.step(1.f/60,{});
        check(g.player.grounded,"Player did not land on inset floor");
        near(g.player.y,floorY);near(g.player.vy,0);
        g.restart();near(g.player.y,floorY);
        for(const auto& e:g.enemies)near(e.y,floorY);
    });
    test("Initial four enemy wave",[]{Combat g;check(g.foes()==4&&g.player.health==100&&!g.ended,"Initial wave");check(g.enemies[0].health==55&&g.enemies[1].health==85,"Enemy types");near(g.enemies[3].x,1285);});
    test("Slime strength increases green red purple",[]{Combat g;check(g.enemies[3].kind==2,"Purple elite missing");check(!g.enemies[3].boss&&g.enemies[3].knockbackResistance()==.65f,"Purple elite must not be boss immune");for(int i=0;i<2;++i){check(slimeSpecs[i].health<slimeSpecs[i+1].health&&slimeSpecs[i].damage<slimeSpecs[i+1].damage,"Tier order");}});
    test("Slime bite waits for contact and hits only once",[]{for(int kind=0;kind<3;++kind){auto g=arena();auto& e=g.enemies[0];e=Enemy(kind,g.player.x+40);g.step(1.f/60,{});check(e.attackAge==0&&g.player.health==100,"Instant bite");const float contact=slimeSpec(kind).attackDuration*slimeSpec(kind).contact;while(e.attackAge+1.f/60<contact)g.step(1.f/60,{});check(g.player.health==100,"Hit before contact");g.step(1.f/60,{});check(g.player.health==100-e.damage,"Missing contact hit");int hp=g.player.health;wait(g,35);check(g.player.health==hp,"Repeated contact");}});
    test("Knockback displaces enemies without interrupting attack windup or recovery",[]{
        for(int kind=0;kind<3;++kind){
            auto g=arena();auto& e=g.enemies[0];
            e=Enemy(kind,g.player.x+40);
            g.step(1.f/60,{});
            check(e.attackAge==0,"Attack did not begin");
            const float timer=e.attackTimer;
            const float initialX=e.x;
            g.hit(e,0,false,0,true);
            check(e.attackAge==0,"Knockback cancelled the active attack");
            check(e.stagger==0,"Knockback applied attack-locking stagger");
            check(e.attackTimer==timer,"Knockback extended enemy attack cooldown");
            check(e.knockback!=0,"Knockback displacement was removed");
            g.step(1.f/60,{});
            check(e.attackAge>0,"Attack failed to advance while knocked back");
            check(e.x!=initialX,"Enemy failed to move during knockback");
        }
    });
    test("Slime telegraph can be escaped and facing stays locked",[]{auto g=arena();auto& e=g.enemies[0];e=Enemy(0,g.player.x+40);g.step(1.f/60,{});check(e.facing==-1,"Facing");g.player.x=e.x+30;wait(g,25);check(g.player.health==100&&e.facing==-1,"Bite hit behind locked jaw");});
    test("Slime animation uses motion and bounded one-shot clips",[]{auto g=arena();auto& e=g.enemies[0];float phase=e.travelPhase;wait(g,8);check(e.travelPhase!=phase,"Walk frozen");check(slimeFrame(0,-1,0)==0&&slimeFrame(.99f,-1,0)==7,"Walk bounds");for(int kind=0;kind<3;++kind){check(slimeFrame(0,0,kind)==8&&slimeFrame(0,slimeSpec(kind).attackDuration*.5f,kind)==12&&slimeFrame(0,100,kind)==15,"Bite atlas range");}});
    test("Animated monster bodies use their outer silhouette in both directions",[]{
        for(int kind=0;kind<3;++kind)for(int frame=0;frame<16;++frame){
            Enemy e(kind,600);e.travelPhase=frame<8?(frame+.1f)/8:0;e.attackAge=frame<8?-1:(frame-8+.1f)/8*slimeSpec(kind).attackDuration;
            e.facing=-1;const auto left=enemyBounds(e);check(left.right-left.left>40,"Body too narrow");
            bool inside=false;enemyAnyBand(e,[&](BodyRect band){const float y=(band.top+band.bottom)*.5f;
                check(enemyOverlaps(e,{band.left+.1f,y,band.left+1,y+.1f}),"Outer visible body not hittable");inside=true;return true;});check(inside,"No body bands");
            e.facing=1;const auto right=enemyBounds(e);near(right.left,1200-left.right);near(right.right,1200-left.left);near(right.top,left.top);near(right.bottom,left.bottom);
        }
    });
    test("Monster empty outline corners do not become rectangular hit targets",[]{
        Enemy e(2,600);e.facing=-1;const auto whole=enemyBounds(e);bool tested=false;
        enemyAnyBand(e,[&](BodyRect band){if(band.left-whole.left<8)return false;
            const float y=(band.top+band.bottom)*.5f;check(!enemyOverlaps(e,{whole.left+.1f,y,whole.left+1,y+.1f}),"Empty corner became solid");tested=true;return true;});
        check(tested,"No narrower silhouette band found");
    });
    test("Sword can hit a monster edge while its center is outside reach",[]{
        auto g=arena();auto& e=g.enemies[0];e=Enemy(0,0);e.speed=0;e.attackTimer=1000;
        const float offset=enemyBounds(e).left;e.x=g.player.x+96-offset-3;
        check(e.x-g.player.x>96,"Center should be beyond legacy reach");Actions a;a.attack=true;g.step(1.f/60,a);wait(g,12);
        check(e.health==43,"Sword missed visible outer body");
    });
    test("Swept projectiles hit animated outer bands without tunneling",[]{
        for(int kind=0;kind<3;++kind){Enemy e(kind,600);bool tested=false;
            enemyAnyBand(e,[&](BodyRect band){const float y=(band.top+band.bottom)*.5f;
                check(enemySweptDisc(e,band.left-200,band.right+200,y,3),"Fast projectile tunneled");
                check(enemySweptDisc(e,band.left-8,band.left-2,y,3),"Projectile radius missed body edge");tested=true;return true;});check(tested,"No projectile band");
            const auto bounds=enemyBounds(e);check(!enemySweptDisc(e,bounds.left-100,bounds.right+100,bounds.top-20,3),"Projectile hit empty air");
        }
    });
    test("Enemy attack distance starts at its visible front edge",[]{
        for(int kind=0;kind<3;++kind){auto g=arena();auto& e=g.enemies[0];e=Enemy(kind,500);e.facing=-1;e.speed=0;
            g.player.x=enemyBounds(e).left-slimeSpec(kind).engageGap-13;
            check(std::abs(e.x-g.player.x)>46,"Must be beyond old center trigger");g.step(1.f/60,{});
            check(e.attackAge==0,"Did not engage from outer edge");
        }
    });
    test("Red slime jumps forward and commits to the telegraphed landing point",[]{
        auto g=arena();auto& e=g.enemies[0];e=Enemy(1,400);g.player.x=300;g.step(1.f/60,{});
        const float target=e.attackToX;check(target<e.attackFromX,"No forward landing target");g.player.x=700;
        float highest=0;bool moved=false,landed=false;for(int i=0;i<70;++i){g.step(1.f/60,{});highest=std::max(highest,floorY-e.y);moved|=e.x<390;
            if(e.attackAge>=slimeSpec(1).attackDuration*slimeSpec(1).contact){near(e.x,target);landed=true;}}
        check(highest>110&&moved&&landed,"No physical leap arc");near(e.y,floorY);check(g.player.health==100,"Homing slam ignored dodge");
    });
    test("Red slam lands once and can be jumped over",[]{
        for(int jump=0;jump<2;++jump){auto g=arena();auto& e=g.enemies[0];e=Enemy(1,400);g.player.x=300;g.step(1.f/60,{});
            const float contact=slimeSpec(1).attackDuration*slimeSpec(1).contact;
            while(e.attackAge+1.f/60<contact){g.step(1.f/60,{});check(g.player.health==100,"Slam hit before landing");}
            if(jump){g.player.y=floorY-65;g.player.grounded=false;g.player.vy=0;}
            g.step(1.f/60,{});const int expected=jump?100:85;check(g.player.health==expected,"Slam landing range/vertical dodge");
            wait(g,20);check(g.player.health==expected,"One slam dealt repeated damage");
        }
    });
    test("Airborne monster collision follows the sprite rather than the floor",[]{
        Enemy e(1,600);e.y=floorY-120;e.attackAge=slimeSpec(1).attackDuration*.45f;
        const auto body=enemyBounds(e);check(body.bottom<floorY-100,"Airborne body stayed at floor");
        check(!enemyOverlaps(e,{500,floorY-20,700,floorY}),"Ground sword hit empty leap shadow");
    });

    test("Air attack selects the downward slam rather than ground combo",[]{
        auto g=arena();g.player.y=floorY-150;g.player.grounded=false;g.player.vy=0;
        auto& e=g.enemies[0];e=Enemy(0,g.player.x+20);e.speed=0;e.attackTimer=1000;
        Actions a;a.attack=true;g.step(1.f/60,a);
        check(g.player.airSlam&&g.player.combo==0&&g.player.meleeLeft==0,"Air attack used grounded combo");
        check(e.health==55&&g.player.slamImpact==0,"Slam damaged before landing");
        Animation animation;animation.step(g.player,1.f/60);check(animation.state==12&&animation.frame>=192,"Air pose not selected");
    });
    test("Air slam accelerates down and deals damage once on actual landing",[]{
        auto g=arena();g.player.y=floorY-180;g.player.grounded=false;g.player.vy=-100;
        auto& e=g.enemies[0];e=Enemy(0,g.player.x+20);e.speed=0;e.attackTimer=1000;
        Actions a;a.attack=true;g.step(1.f/60,a);bool fast=false;int frames=0;
        while(g.player.airSlam&&frames++<60){g.step(1.f/60,{});fast|=g.player.vy>=airSlamDropSpeed;if(!g.player.grounded)check(e.health==55,"Damage before ground contact");}
        check(fast&&frames<60&&g.player.grounded,"Downward physics failed");
        check(e.health==31&&g.player.slamImpact>0&&g.player.slamRecover>0,"Landing strike missing");
        wait(g,45);check(e.health==31,"Slam dealt repeated damage");
    });
    test("Attack just before contact is rejected and does not buffer",[]{
        auto g=arena();g.player.y=floorY-1;g.player.grounded=false;g.player.vy=80;
        auto& e=g.enemies[0];e=Enemy(0,g.player.x+20);e.speed=0;e.attackTimer=1000;
        Actions a;a.attack=true;g.step(1.f/60,a);check(g.player.grounded&&!g.player.airSlam&&g.player.slamImpact==0,"Attack started too near the floor");
        wait(g,15);check(e.health==55&&g.player.attackSerial==0,"Rejected attack fired on landing");
    });
    test("Air slam cancels on dash swap skill damage and restart without ghost hit",[]{
        for(int mode=0;mode<5;++mode){auto g=arena();g.player.y=floorY-220;g.player.grounded=false;g.player.vy=0;
            auto& e=g.enemies[0];e=Enemy(0,g.player.x+20);e.speed=0;e.attackTimer=1000;
            Actions a;a.attack=true;g.step(1.f/60,a);a={};
            if(mode==0)a.dash=true;else if(mode==1)a.swap=true;else if(mode==2)a.skill=true;else if(mode==3)g.damagePlayer(1);else a.restart=true;
            g.step(1.f/60,a);check(!g.player.airSlam,"Slam did not cancel");
            if(mode==4){check(g.player.slamImpact==0&&g.player.slamRecover==0,"Restart retained slam");continue;}
            wait(g,60);check(e.health==55&&g.player.slamImpact==0,"Cancelled slam left delayed damage");
        }
    });
    test("Landing impact follows the blade tip for both facings and gear",[]{
        for(int facing:{-1,1}){auto g=arena();g.player.x=640;g.player.y=floorY-100;g.player.grounded=false;g.player.facing=facing;g.player.gear.selected[0]=1;
            auto& e=g.enemies[0];e=Enemy(0,640+facing*20);e.speed=0;e.attackTimer=1000;Actions a;a.attack=true;g.step(1.f/60,a);
            for(int i=0;i<60&&g.player.airSlam;++i)g.step(1.f/60,{});
            near(g.player.slamImpactX,g.player.x+facing*(slamContactTipX-motionPivotX)*motionScale);
            near(g.player.slamImpactY,floorY+(slamContactTipY-motionPivotY)*motionScale);check(e.health==25,"Slam ignored weapon damage");
        }
    });
    test("Ground slam wave misses airborne bodies and mage air attacks remain projectiles",[]{
        auto g=arena();g.player.y=floorY-90;g.player.grounded=false;
        auto& e=g.enemies[0];e=Enemy(0,g.player.x+20);e.y=floorY-130;e.speed=0;e.attackTimer=1000;
        Actions a;a.attack=true;g.step(1.f/60,a);for(int i=0;i<60&&g.player.airSlam;++i)g.step(1.f/60,{});
        check(e.health==55&&g.player.slamImpact>0,"Ground wave hit empty space under airborne enemy");
        auto mage=arena();mage.player.form=1;mage.player.grounded=false;mage.player.y=floorY-100;mage.step(1.f/60,a);
        check(!mage.player.airSlam&&mage.projectiles.empty(),"Mage attack skipped the staff wind-up");
        wait(mage,8);
        check(mage.projectiles.size()==1,"Mage air attack did not release a fireball");
    });
    test("Repeated air attack input cannot restart the falling slam",[]{
        auto g=arena();g.player.y=floorY-900;g.player.grounded=false;Actions a;a.attack=true;
        g.step(1.f/60,a);const int serial=g.player.attackSerial;
        for(int i=0;i<35;++i){g.step(1.f/60,a);check(g.player.attackSerial==serial,"Slam restarted while falling");}
    });
    test("Slam animation holds descent and contact then recovers without ghost combo",[]{
        Player p;p.grounded=false;p.airSlam=true;p.slamAge=.5f;++p.attackSerial;Animation a;
        for(int i=0;i<60;++i){a.step(p,1.f/60);}
        check(a.state==12&&a.frame==196,"Descent did not hold");
        p.airSlam=false;p.grounded=true;p.slamRecover=airSlamRecovery;p.hitStop=.065f;a.step(p,1.f/60);const int contact=a.frame;
        a.step(p,1.f/60);check(contact==197&&a.frame==contact,"Slam hit-stop lost contact pose");
        p.hitStop=0;p.slamRecover=.02f;a.step(p,1.f/60);check(a.frame==199,"Recovery frame missing");
        p.slamRecover=0;for(int i=0;i<20;++i)a.step(p,1.f/60);check(a.state==0,"Phantom grounded attack after slam");
    });

    test("Movement facing walls",[]{auto g=arena();Actions a;a.right=true;g.step(1.f/60,a);near(g.player.x,140+280.f/60);g.player.x=1549;g.step(1.f/60,a);near(g.player.x,1550);a={};a.left=true;g.player.x=26;g.step(1.f/60,a);near(g.player.x,25);check(g.player.facing== -1,"Facing");});
    test("Held touch jumps again on the contact tick and stops after release",[]{
        Input in;auto g=arena();Animation anim;
        in.pointer(1,99,610,true);
        g.step(1.f/60,in.sample());
        check(!g.player.grounded&&g.player.vy<0,"Initial touch jump missing");
        for(int i=0;i<100&&g.player.landingSerial==0;++i)g.step(1.f/60,in.sample());
        check(g.player.landingSerial==1&&!g.player.grounded&&g.player.vy<0,"Contact did not immediately launch another jump");
        anim.step(g.player,1.f/60);check(anim.state==2,"Bounce incorrectly showed a landing pose");
        in.pointer(1,99,610,false);
        for(int i=0;i<100;++i)g.step(1.f/60,in.sample());
        check(g.player.grounded&&g.player.landingSerial==2,"Jump continued after stick release");
    });
    test("One-way ledge landing uses crossed surface instead of world floor",[]{
        auto g=arena();g.platforms={{100,200,floorY-70},{100,200,floorY-100}};
        Actions a;a.jump=a.attack=true;g.step(1.f/60,a);
        check(g.player.attackSerial==0,"Attack started on takeoff");
        a={};a.attack=true;g.step(1.f/60,a);
        check(g.player.attackSerial==0,"Takeoff lock was skipped");
        wait(g,80);
        near(g.player.y,floorY-100);check(g.player.grounded&&g.player.landingSerial==1,"Missed upper ledge contact");
        a={};a.right=true;
        for(int i=0;i<45;++i)g.step(1.f/60,a);
        near(g.player.y,floorY);check(g.player.landingSerial==2,"Walking off ledge lost the lower landing event");
    });
    test("Jump apex and landing",[]{auto g=arena();Actions a;a.jump=true;g.step(1.f/60,a);check(!g.player.grounded&&g.player.vy<0,"Jump");float highest=g.player.y;for(int i=0;i<90;++i){g.step(1.f/60,{});highest=std::min(highest,g.player.y);}check(floorY-highest>160&&floorY-highest<180,"Apex");check(g.player.grounded&&g.player.vy==0,"Landing");});
    test("Dash immunity and cooldown",[]{auto g=arena();Actions a;a.dash=true;g.step(1.f/60,a);near(g.player.vx,1150);g.damagePlayer(15);check(g.player.health==100,"Dash immunity");wait(g,12);float cooldown=g.player.dashCooldown;g.step(1.f/60,a);check(g.player.dashLeft==0&&g.player.dashCooldown<cooldown,"Cooldown");wait(g,60);g.step(1.f/60,a);check(g.player.dashLeft>0,"Reactivation");});
    test("Three hit combo damage lands at the blade contact",[]{auto g=arena();auto& e=g.enemies[0];Actions a;a.attack=true;int health=55;
        for(int combo=1;combo<=3;++combo){e.x=200;g.step(1.f/60,a);check(e.health==health&&g.player.combo==combo,"Hit skipped wind-up");wait(g,combo==3?14:13);health-=combo==1?12:combo==2?15:23;check(e.health==health,"Contact damage");if(combo==3)check(e.bleedStacks==2&&g.player.preparation==0,"Passive finisher bleed requires no buff");wait(g,20);}
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
    test("Mage empowered projectile collision",[]{auto g=arena();g.player.form=1;Actions a;a.skill=true;g.step(1.f/60,a);check(g.projectiles.size()==1&&g.projectiles[0].empowered,"Skill");near(g.player.mageSkillCooldown,4.1f);g.enemies[0].x=230;wait(g,12);check(g.enemies[0].health==30&&g.enemies[0].burn>0,"Collision and burn");});
    test("Thrust selects nearest live target in either facing",[]{
        for(int facing:{-1,1}){
            auto g=arena();g.player.x=600;g.player.facing=facing;
            g.enemies[0]=Enemy(0,600+facing*115);
            g.enemies[1]=Enemy(0,600+facing*70);
            for(auto& e:g.enemies){e.speed=0;e.attackTimer=1000;}
            Actions a;a.skill=true;g.step(1.f/60,a);
            check(g.enemies[1].health==55,"Thrust skipped wind-up");
            wait(g,16);
            check(g.enemies[0].health==55&&g.enemies[1].health==23&&g.enemies[1].bleedStacks==2,"Thrust hit more than the nearest target");
        }
    });
    test("Sweep spreads at most once and cannot relay through a new target",[]{
        auto g=arena();g.player.x=500;
        g.enemies={Enemy(0,550),Enemy(0,590),Enemy(0,690),Enemy(0,775)};
        for(auto& e:g.enemies){e.speed=0;e.attackTimer=1000;}
        g.hit(g.enemies[0],0,false,2);g.hit(g.enemies[1],0,false,2);
        for(auto& e:g.enemies)e.knockback=0;
        Actions a;a.skillSweep=true;g.step(1.f/60,a);wait(g,14);
        check(g.enemies[0].health==37&&g.enemies[1].health==37,"Sweep failed its direct area hit");
        check(g.enemies[2].health==55&&g.enemies[2].bleedStacks==1&&g.enemies[3].bleedStacks==0,"Spread incorrectly relayed through a new source");
        check(g.enemies[0].bleedStacks==3&&g.enemies[1].bleedStacks==3,"Spread applied more than once");
    });
    test("Bleed milestones slow at five and execute only regular enemies at ten",[]{
        auto g=arena();
        g.enemies[0]=Enemy(0,700);g.enemies[1]=Enemy(0,700);
        for(int i=0;i<2;++i){g.enemies[i].attackTimer=1000;g.hit(g.enemies[i],0,false,4+i);g.enemies[i].knockback=0;}
        g.step(1.f/60,{});
        near(700-g.enemies[1].x,(700-g.enemies[0].x)*.8f);
        for(int i=0;i<2;++i){
            auto& e=g.enemies[i];e.health=i==0?12:15;e.boss=i==1;e.speed=0;
            g.hit(e,0,false,10);check(e.bleedStacks==10,"Cap not enforced");
        }
        wait(g,46);
        check(g.enemies[0].dead()&&g.enemies[1].health==5,"Incorrect regular or boss maximum-stack effect");
    });
    test("Preparation accelerates basic contact and keeps skill cooldowns separate",[]{
        auto g=arena();Actions a;a.skillBuff=true;a.attack=true;
        g.step(1.f/60,a);
        near(g.player.preparation,5.f);near(g.player.buffReady,14.f);
        near(g.player.attackCooldown,.23f/1.2f);
        check(g.player.skillCooldown==0&&g.player.sweepReady==0,"Preparation consumed another cooldown");
        wait(g,302);check(g.player.preparation==0&&g.player.basicSpeed()==1,"Preparation did not expire");
    });
    test("Three skill buttons dispatch distinct touch edges",[]{
        Input in;ControlLayout ui;in.setLayout(&ui);
        for(int i=0;i<3;++i){
            const auto& b=ui.buttons[ControlLayout::Skill+i];
            in.pointer(i,b.x+b.size/2,b.y+b.size/2,true);
            auto a=in.sample();
            check(a.skill==(i==0)&&a.skillSweep==(i==1)&&a.skillBuff==(i==2),"Wrong touch skill");
            in.pointer(i,b.x,b.y,false);
        }
    });
    test("Nine item stats health adjustment",[]{Player p;p.equip(0);p.equip(1);check(p.gear.attack()==6&&p.gear.defense()==4&&p.maxHealth()==130&&p.health==130,"Armor weapon");near(p.gear.speed(),.792f);near(p.gear.range(),1.16f);p.equip(2);check(p.gear.attack()==9,"Flame charm");p.equip(1);check(p.health==110&&p.maxHealth()==110,"Health adjustment");p.equip(2);check(p.gear.defense()==3&&p.maxHealth()==125,"Guardian charm");});
    test("Damage immunity defeat freeze",[]{auto g=arena();g.damagePlayer(9);g.damagePlayer(9);check(g.player.health==91,"Damage immunity");wait(g,45);g.damagePlayer(999);check(g.ended&&!g.won&&g.player.health==0,"Defeat");float x=g.player.x;Actions a;a.right=true;g.step(1.f/60,a);near(g.player.x,x);});
    test("Stage clear",[]{auto g=arena();for(auto& e:g.enemies)g.hit(e,1000,false);g.step(1.f/60,{});check(g.ended&&g.won&&g.foes()==0,"Clear");});
    test("Restart clears gear projectiles effects",[]{auto g=arena();g.player.form=1;g.player.equip(0);Actions a;a.attack=true;g.step(1.f/60,a);g.hit(g.enemies[0],1,true);a={};a.restart=true;g.step(1.f/60,a);check(g.foes()==4&&g.projectiles.empty()&&g.popups.empty()&&!g.ended&&g.player.form==0&&g.player.gear.selected[0]==0,"Restart state");});
    test("Rear and vertical melee excluded",[]{auto g=arena();g.enemies[0].x=g.player.x-200;Actions a;a.attack=true;g.step(1.f/60,a);check(g.enemies[0].health==55,"Rear hit");wait(g,18);g.player.y=400;g.player.grounded=false;g.enemies[0].x=200;a={};a.skill=true;g.step(1.f/60,a);check(g.enemies[0].health==55,"Vertical hit");});
    test("Multitouch independent keyboard ownership",[]{Input in;in.pointer(1,60,650,true);in.pointer(2,1160,590,true);auto a=in.sample();check(a.left&&a.attack,"Move plus attack");check(!in.sample().attack,"Held attack edge");in.pointer(2,0,0,false);check(in.sample().left,"Attack up retains movement");Actions k;k.right=true;in.keyboard(k);in.pointer(1,0,0,false);check(in.sample().right,"Keyboard retained");in.reset();check(!in.sample().right,"Reset");});
    test("Fast tap buffered between simulation steps",[]{Input in;in.pointer(1,1160,590,true);in.pointer(1,0,0,false);check(in.sample().attack,"Fast touch lost");check(!in.sample().attack,"Single consume");});
    test("Joystick upward jump and focus reset",[]{Input in;in.pointer(1,99,610,true);check(in.sample().jump,"Up jump");auto held=in.sample();check(!held.jump&&held.jumpHeld,"Held up must survive sampling without another edge");in.reset();check(!in.sample().jump,"Focus clear");});
    test("Inventory equip and retry touch targets",[]{Input in;in.pointer(1,800,40,true);check(in.sample().inventory,"Inventory");in.reset();in.pointer(2,90,430,true,false,true);check(in.sample().weapon,"Weapon");in.reset();in.pointer(3,600,440,true,true,false);check(in.sample().restart,"Retry");});
    test("Animation attack retrigger and death bounds",[]{Player p;Animation a;++p.attackSerial;p.combo=3;a.step(p,1.f/60);check(a.frame>=144&&a.frame<154,"Third attack atlas");p.health=0;for(int i=0;i<100;++i)a.step(p,1.f/60);check(a.frame==187,"Death last frame");});
    test("Every combo reaches its final frame and recovers",[]{for(int combo=1;combo<=3;++combo){Player p;p.combo=combo;++p.attackSerial;Animation a;int largest=0;for(int i=0;i<70;++i){a.step(p,1.f/60);if(a.state==3+combo)largest=std::max(largest,a.frame);}const auto clip=motionClips[3+combo];check(largest==clip.start+clip.count-1&&a.state==0,"Truncated clip");}});
    test("Run cycle follows actual world travel without wall skating",[]{Player p;p.vx=280;Animation a;a.step(p,1.f/60);int first=a.frame;for(int i=0;i<16;++i){p.x+=7.5f;a.step(p,1.f/60);}check(a.frame==first,"Stride distance mismatch");for(int i=0;i<30;++i)a.step(p,1.f/60);check(a.frame==first,"Stationary feet animated");});
    test("Jump apex fall and landing use distinct clips",[]{Player p;Animation a;p.grounded=false;p.vy=-650;a.step(p,1.f/60);check(a.state==2&&a.frame==48,"Takeoff");p.vy=350;a.step(p,1.f/60);check(a.state==3&&a.frame>=64,"Fall");p.grounded=true;p.vy=0;++p.landingSerial;a.step(p,1.f/60);check(a.state==10,"Landing missing");for(int i=0;i<12;++i)a.step(p,1.f/60);check(a.state==0,"Landing stuck");});
    test("Hit-stop holds the contact pose then resumes",[]{Player p;p.combo=2;++p.attackSerial;p.hitStop=.075f;Animation a;a.step(p,1.f/60);const int contact=a.frame;check(contact>=133,"Hit froze before contact");a.step(p,1.f/60);check(a.frame==contact,"Hit-stop pose advanced");p.hitStop=0;a.step(p,.05f);check(a.frame>contact,"Contact did not resume");});
    test("Forest entrance authored PNG contract",[]{
        check(ForestLayerScene::layersEntrance.size()==6,"Entrance layer count");
        constexpr float parallax[]={0.f,.06f,.14f,.40f,1.f,1.f};
        for(size_t i=0;i<ForestLayerScene::layersEntrance.size();++i){
            check(ForestLayerScene::layersEntrance[i].parallax==parallax[i],
                  "Entrance layer parallax mismatch");
            check(ForestLayerScene::layersEntrance[i].file!=nullptr,
                  "Entrance file path missing");
            if(i>=4)check(ForestLayerScene::layersEntrance[i].worldLocked,
                          "Entrance collision-floor image moved");
        }
        check(ForestLayerScene::floorY==597,"Forest entrance gameplay floor changed");
        check(ForestLayerScene::worldWidth==1600,"Entrance world width changed");
    });
    test("Forest v3 keeps six distinct visual-only layers",[]{
        check(ForestLayerScene::layersV3.size()==6,"v3 PNG layer count");
        constexpr float expected[]={0.f,.06f,.14f,.40f,1.f,1.f};
        for(size_t i=0;i<ForestLayerScene::layersV3.size();++i){
            check(ForestLayerScene::layersV3[i].parallax==expected[i],
                "Incorrect v3 parallax");
            check(ForestLayerScene::layersV3[i].file!=nullptr,
                "Missing v3 layer path");
            if(i>=4)check(ForestLayerScene::layersV3[i].worldLocked,
                "Foreground moved away from NPC coordinates");
        }
        check(ForestLayerScene::floorY==597 &&
              ForestLayerScene::worldWidth==1600,"Changed gameplay geometry");
        for(auto zone:ForestLayerScene::npcZones)
            for(int x=zone.x0;x<=zone.x1;++x)
                check(!ForestLayerScene::canPlaceForegroundProp(x),
                    "Decor could occupy protected NPC corridor");
    });
    test("Layered forest keeps ground and NPC regions deterministic",[]{
        check(ForestLayerScene::floorY==597,"Collision floor moved");
        check(ForestLayerScene::worldWidth==1600,"World boundaries changed");
        check(ForestLayerScene::layers.size()==4,"Four layer contract");
        check(ForestLayerScene::layers[0].parallax==0.f,"Sky must stay stationary");
        check(ForestLayerScene::layers[3].parallax==1.f,"Ground must follow world");
        for(auto zone:ForestLayerScene::npcZones){
            check(zone.x1>zone.x0,"Invalid NPC zone");
            for(int x=zone.x0;x<=zone.x1;++x){
                check(ForestLayerScene::npcSafe(x),"NPC reservation failed");
                check(!ForestLayerScene::canPlaceForegroundProp(x),"Prop in NPC reserved area");
            }
        }
        for(uint32_t seed=0;seed<100;seed++){
            check(ForestLayerScene::farTint(seed)>=239u &&
                  ForestLayerScene::farTint(seed)<=255u,"Too much far layer variation");
        }
        check(ForestLayerScene::canPlaceForegroundProp(60),"Everything reserved");
        check(!ForestLayerScene::canPlaceForegroundProp(260),"Quest corridor blocked");
    });
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

    test("Story tutorial opens as the Flash Executioner in a live fight, no text, beacon first",[]{
        StoryDirector d;Combat g;d.start(g);
        check(d.stage==StoryStage::Practice,"Story must open with the combat tutorial");
        check(g.player.flashExecutioner&&g.player.activeJob==0,"The prologue must start as the Flash Executioner");
        tickStory(d,g,Actions{},3);
        check(d.view.highlight==ControlLayout::Stick&&d.view.pipsTotal==StoryDirector::stepCount,"Move prompt missing");
        check(d.view.goalX>0,"The first beat needs a beacon to walk to");check(d.view.pips==0,"Nothing done yet, no progress");
    });
    test("Story tutorial: a bot answers every prompt and time stop, then the castle threshold opens",[]{
        StoryDirector d;Combat g;d.start(g);
        bool sawFreeze=false;int tick=0;
        while(d.stage==StoryStage::Practice&&tick<60*900){
            Actions a;const int c=d.view.highlight;
            if(d.view.freeze>0)sawFreeze=true;
            if(c==ControlLayout::Stick){if(d.view.arrow==2)a.jump=tick%2==0;else a.right=true;}
            else if(c>=0&&tick%2==0){
                if(c==ControlLayout::Dash)a.dash=true;
                else if(c==ControlLayout::Attack)a.attack=true;
                else if(c==ControlLayout::Skill)a.skill=true;
                else if(c==ControlLayout::Sweep)a.skillSweep=true;
                else if(c==ControlLayout::Buff)a.skillBuff=true;
                else if(c==ControlLayout::Ultimate)a.ultimate=true;
                else if(c==ControlLayout::Swap)a.swap=true;
            }
            if(d.view.freeze<=0&&c!=ControlLayout::Stick){   // close in on the target (or the nearest foe)
                float tx=d.view.hintX;
                if(tx<0){float best=1e9f;for(const auto& e:g.enemies)if(!e.dead()&&std::abs(e.x-g.player.x)<best){best=std::abs(e.x-g.player.x);tx=e.x;}}
                if(tx>=0&&std::abs(tx-g.player.x)>90){a.right=tx>g.player.x;a.left=!a.right;}
            }
            tickStory(d,g,a,1);++tick;
        }
        check(d.stage==StoryStage::Threshold,"The bot never finished the tutorial");
        check(sawFreeze,"Reaction beats must stop time");
        check(!d.view.skipVisible&&g.timeScale==1.f,"Skip control and the time stop must be gone after the tutorial");
        check(g.player.flashExecutioner&&g.enemies.empty(),"The tutorial must hand back the Flash Executioner and a clean field");
    });
    test("Story time stop freezes the fight until the prompted button, and the dodge goes through the blow",[]{
        StoryDirector d;Combat g;d.start(g);
        g.player.x=d.view.goalX;tickStory(d,g,Actions{},40);                 // walk beat done
        Actions j;j.jump=true;tickStory(d,g,j,1);tickStory(d,g,Actions{},60); // jump beat done
        int guard=0;while(d.view.freeze<=0&&guard++<60*20)tickStory(d,g,Actions{},1);
        check(d.view.freeze>0&&g.timeScale==0.f,"The soldier's wind-up must stop time");
        const float age=g.enemies[0].attackAge;Actions wrong;wrong.attack=true;tickStory(d,g,wrong,5);
        check(g.enemies[0].attackAge==age&&d.view.freeze>0,"Only the prompted button resumes time");
        Actions dash;dash.dash=true;tickStory(d,g,dash,1);
        check(g.timeScale==1.f&&g.player.dashLeft>0,"The dash must fire straight out of the time stop");
        const int hurt=g.player.hurtSerial;tickStory(d,g,Actions{},60);
        check(g.player.hurtSerial==hurt,"Dashing out of the time stop must dodge the blow");
    });
    test("Story tutorial: only the beat's own button works, every other action is dead",[]{
        StoryDirector d;Combat g;d.start(g);
        Actions all;all.attack=all.skill=all.skillSweep=all.skillBuff=all.ultimate=all.swap=all.dash=true;
        tickStory(d,g,Actions{},3);
        const int job=g.player.activeJob;
        tickStory(d,g,all,30);
        check(g.player.activeJob==job&&g.player.dashCooldown==0.f&&g.player.flashMove==FlashMove::None,"Walk beat must ignore every action button");
        check(g.player.storedLightLances==0&&d.view.pips==0,"Wrong buttons must not progress or spend anything");
    });
    // A bare field with passive foes at chosen x positions; nothing attacks, nothing moves on its own.
    auto lightDarkField=[](std::initializer_list<float> xs){
        Combat g;g.scriptedSpawns=true;g.endOnClear=false;g.configureCastleArea(1);g.enemies.clear();
        g.player.x=300;g.player.equippedJobs={{1,2}};
        for(float x:xs){g.spawn(AshInfantry,x,0,1.f);auto& e=g.enemies.back();e.maxHealth=e.health=900;e.attackTimer=1e9f;e.speed=0;}
        return g;
    };
    test("Dark stacks: Ruler direct hits add them (bolt 1, spikes 2, repulsion 1), caps at 5, fades after 6 s",[]{
        Combat g;g.scriptedSpawns=true;g.endOnClear=false;g.configureCastleArea(1);g.enemies.clear();
        g.spawn(AshInfantry,420,0,1.f);g.enemies[0].maxHealth=g.enemies[0].health=900;g.enemies[0].attackTimer=1e9f;
        g.elementHit(g.enemies[0],10,tags::rulerBolt);check(g.enemies[0].darkStacks==1,"A bolt hit adds one dark stack");
        g.elementHit(g.enemies[0],10,tags::rulerSpikes);check(g.enemies[0].darkStacks==3,"Spikes add two");
        for(int i=0;i<5;++i)g.elementHit(g.enemies[0],1,tags::rulerSpikes);
        check(g.enemies[0].darkStacks==stackMax,"Dark stacks cap at five");
        for(int i=0;i<60*7;++i)g.step(1.f/60,{});
        check(g.enemies[0].darkStacks==0,"Dark stacks fade after six seconds");
    });
    test("Singularity eats dark stacks: +30 % damage per stack, stacks consumed",[]{
        Combat a;a.scriptedSpawns=true;a.endOnClear=false;a.configureCastleArea(1);a.enemies.clear();
        Combat b=a;
        for(Combat* g:{&a,&b}){g->player.x=300;g->spawn(AshInfantry,420,0,1.f);auto& e=g->enemies[0];e.maxHealth=e.health=900;e.attackTimer=1e9f;}
        b.enemies[0].darkStacks=4;b.enemies[0].darkTimer=6.f;
        for(Combat* g:{&a,&b}){g->player.form=1;g->player.flashExecutioner=false;g->player.equippedJobs={{1,2}};
            Actions u;u.ultimate=true;for(int i=0;i<110;++i){g->step(1.f/60,u);u={};}}
        const int plain=900-a.enemies[0].health,stacked=900-b.enemies[0].health;
        check(plain>0&&stacked>plain*2,"Four stacks must at least double the singularity's damage");
        check(b.enemies[0].darkStacks==0,"The singularity consumes the stacks");
    });
    test("Eclipse: a Ruler hit on a light-stacked foe absorbs nearby light, pulls foes in, then collapses",[lightDarkField]{
        Combat g=lightDarkField({420,520,610,1500});
        for(int i=0;i<3;++i)g.enemies[i].lightMark=5.f;
        g.elementHit(g.enemies[0],10,tags::rulerBolt);
        check(g.eclipseFx.size()==1&&g.eclipseFx[0].kind==0&&g.eclipseFx[0].marks==3,"Three light stacks are absorbed into one eclipse");
        check(g.enemies[0].lightMark<=0&&g.enemies[1].lightMark<=0&&g.enemies[2].lightMark<=0,"The light stacks are consumed");
        const float far=g.enemies[2].x;
        for(int i=0;i<30;++i)g.step(1.f/60,{});
        check(g.enemies[2].x<far-20,"The eclipse pulls foes toward its centre");
        check(g.enemies[3].x==1500,"Foes outside the pull are untouched");
        const int before=g.enemies[0].health;
        for(int i=0;i<40;++i)g.step(1.f/60,{});
        check(g.enemies[0].health<before-40,"The collapse deals the blast");
        for(int i=0;i<30;++i)g.step(1.f/60,{});
        check(g.eclipseFx.empty(),"The eclipse ends");
    });
    test("Eclipse needs a light stack; light foes are not re-triggered during the cooldown",[lightDarkField]{
        Combat g=lightDarkField({420});
        g.elementHit(g.enemies[0],10,tags::rulerBolt);check(g.eclipseFx.empty(),"No light stack, no eclipse");
        g.enemies[0].lightMark=5.f;g.elementHit(g.enemies[0],10,tags::rulerBolt);check(g.eclipseFx.size()==1,"Light stack plus dark hit starts one");
        g.enemies[0].lightMark=5.f;g.elementHit(g.enemies[0],10,tags::rulerBolt);check(g.eclipseFx.size()==1,"The reaction cooldown stops a re-trigger");
    });
    test("Solar burst: a Flash hit on dark-stacked foes flashes the stacks into a blast; neighbours are hit too",[lightDarkField]{
        Combat g=lightDarkField({420,500,1500});
        g.enemies[0].darkStacks=4;g.enemies[0].darkTimer=6.f;
        const int n0=g.enemies[1].health,far0=g.enemies[2].health;
        g.elementReact(g.enemies[0],tags::flashStrike);
        check(g.enemies[0].darkStacks==0&&g.eclipseFx.size()==1&&g.eclipseFx[0].kind==1,"The stacks are consumed into a solar burst");
        check(g.enemies[1].health<n0&&g.enemies[2].health==far0,"The blast hits close neighbours only");
        check(900-g.enemies[0].health==20+16*4,"Damage scales with the consumed stacks");
        g.enemies[0].darkStacks=3;g.elementReact(g.enemies[0],tags::flashStrike);check(g.enemies[0].darkStacks==3,"The cooldown blocks an instant repeat");
    });
    test("Void spikes execute non-boss foes below 6 % health per void stack; bosses are spared",[lightDarkField]{
        Combat g=lightDarkField({420,440,460});
        g.enemies[2].boss=true;
        for(auto& e:g.enemies){e.maxHealth=e.health=1000;e.darkStacks=3;e.darkTimer=6.f;}
        g.enemies[0].health=190;   // 3 stacks (+2 from the spikes, capped) = 5 stacks: executes at 30 % = 300
        g.enemies[1].health=500;   // above the threshold
        g.enemies[2].health=190;   // a boss is never executed
        g.player.form=1;g.player.flashExecutioner=false;g.player.equippedJobs={{1,2}};g.player.facing=1;
        Actions u;u.skillSweep=true;for(int i=0;i<40;++i){g.step(1.f/60,u);u={};}
        check(g.enemies[0].dead(),"A non-boss under the void threshold is executed by the spikes");
        check(!g.enemies[1].dead(),"A foe above the threshold survives");
        check(!g.enemies[2].dead(),"Bosses are never executed");
    });
    test("Void repulsion pushes harder with more void stacks",[lightDarkField]{
        float pushed[2]={0,0};
        for(int k=0;k<2;++k){
            Combat g=lightDarkField({380});
            g.enemies[0].maxHealth=g.enemies[0].health=9000;
            if(k==1){g.enemies[0].darkStacks=3;g.enemies[0].darkTimer=6.f;}
            g.player.form=1;g.player.flashExecutioner=false;g.player.equippedJobs={{1,2}};
            Actions b;b.skillBuff=true;for(int i=0;i<30;++i){g.step(1.f/60,b);b={};}
            pushed[k]=g.enemies[0].x-380;
        }
        check(pushed[0]>0&&pushed[1]>pushed[0]*1.4f,"Three void stacks must push clearly farther");
    });
    test("Void stacks: 3 slow, 5 erode (freeze + damage over time; bosses keep moving but still erode)",[lightDarkField]{
        float moved[3]={0,0,0};int hp[3]={0,0,0};
        for(int k=0;k<3;++k){
            Combat g=lightDarkField({1000});
            auto& e=g.enemies[0];e.speed=100;e.maxHealth=e.health=1000;e.attackTimer=1e9f;
            if(k==1){e.darkStacks=3;e.darkTimer=60.f;}
            if(k==2){e.darkStacks=5;e.darkTimer=60.f;}
            const float x0=e.x;const int h0=e.health;
            for(int i=0;i<60;++i)g.step(1.f/60,{});
            moved[k]=x0-g.enemies[0].x;hp[k]=h0-g.enemies[0].health;
        }
        check(moved[0]>50&&moved[1]<moved[0]*.75f&&moved[1]>0,"Three void stacks slow the walk");
        check(moved[2]==0.f&&hp[2]>0&&hp[0]==0,"Five void stacks freeze the foe and eat it from inside");
        Combat g=lightDarkField({1000});auto& b=g.enemies[0];b.boss=true;b.speed=100;b.maxHealth=b.health=1000;b.darkStacks=5;b.darkTimer=60.f;
        const float x0=b.x;for(int i=0;i<60;++i)g.step(1.f/60,{});
        check(g.enemies[0].x<x0-20&&g.enemies[0].health<1000,"A boss is not frozen by erosion but still erodes");
    });
    test("Elements are character-agnostic: any hit tagged light or dark reacts, untagged hits never do",[lightDarkField]{
        Combat g=lightDarkField({420,430});
        const ElementTag newLight{Element::Light,2},newDark{Element::Dark,1};   // a future job's tags
        g.elementHit(g.enemies[0],5,newLight);
        check(Combat::stacksOf(g.enemies[0],Element::Light)==2,"A light tag leaves its stacks");
        g.elementHit(g.enemies[0],5,newLight);g.elementHit(g.enemies[0],5,newLight);g.elementHit(g.enemies[0],5,newLight);
        check(Combat::stacksOf(g.enemies[0],Element::Light)==stackMax,"Light stacks cap too");
        g.elementHit(g.enemies[1],5,tags::none);
        check(g.eclipseFx.empty()&&Combat::stacksOf(g.enemies[1],Element::Dark)==0,"An untagged hit adds nothing and reacts with nothing");
        g.elementHit(g.enemies[0],5,newDark);
        check(g.eclipseFx.size()==1&&g.eclipseFx[0].kind==0&&g.eclipseFx[0].marks==stackMax,"A new job's dark hit eclipses all five light stacks");
        check(Combat::stacksOf(g.enemies[0],Element::Light)==0&&Combat::stacksOf(g.enemies[0],Element::Dark)==1,"Light consumed, the dark stack sticks");
    });
    test("Light stacks fade after six seconds and count past one",[lightDarkField]{
        Combat g=lightDarkField({420});
        g.elementHit(g.enemies[0],5,tags::flashLance);g.elementHit(g.enemies[0],5,tags::flashLance);
        check(Combat::stacksOf(g.enemies[0],Element::Light)==2,"Command lances stack light");
        for(int i=0;i<60*7;++i)g.step(1.f/60,{});
        check(Combat::stacksOf(g.enemies[0],Element::Light)==0&&g.enemies[0].lightStacks==0,"Light stacks fade");
    });
    test("Flash basic hit on a dark-stacked foe really triggers the solar burst in combat",[lightDarkField]{
        Combat g=lightDarkField({340});
        g.player.flashExecutioner=true;g.player.form=0;g.player.activeJob=0;g.player.equippedJobs={{2,1}};g.refreshStats();g.player.facing=1;
        g.enemies[0].darkStacks=3;g.enemies[0].darkTimer=6.f;
        Actions a;a.attack=true;for(int i=0;i<60;++i){g.step(1.f/60,a);if(g.enemies[0].darkStacks==0)break;}
        check(g.enemies[0].darkStacks==0,"A Flash sword hit must flash the dark stacks away");
    });
    test("Practice dummies never attack and never fall, and count the hits they take",[]{
        StoryDirector d;Combat g;d.start(g);
        tickStory(d,g,Actions{},2);
        g.spawn(AshInfantry,g.player.x+60,0,1.f);
        auto& e=g.enemies.back();e.dummy=true;e.health=e.maxHealth=100;
        for(int i=0;i<20;++i)g.hit(g.enemies.back(),90,false);
        check(!g.enemies.back().dead()&&g.enemies.back().dummyHits==20,"A dummy must survive and count every hit");
        tickStory(d,g,Actions{},60);
        check(g.player.health==g.player.maxHealth(),"A dummy must never hurt the player");
    });
    test("Story skip button jumps straight past practice",[]{
        StoryDirector d;Combat g;d.start(g);
        check(StoryDirector::skipHit(StoryDirector::skipX+5,StoryDirector::skipY+5),"Skip hitbox");
        d.requestSkip(g);
        check(d.stage==StoryStage::Threshold,"Skip did not leave practice");
    });
    test("Story waves seal the exit until cleared and never raise STAGE CLEAR",[]{
        StoryDirector d;Combat g;d.start(g);d.requestSkip(g);
        g.player.x=1500;tickStory(d,g,Actions{},2);
        check(d.stage==StoryStage::FirstFoes&&g.enemies.size()==6,"A1 wave should be 4 infantry + 2 gargoyles");
        check(!d.portalOpen(),"Exit must stay sealed while foes remain");
        clearFoes(g);tickStory(d,g,Actions{},2);
        check(d.portalOpen(),"Clearing the room should open the exit");
        check(!g.ended,"Tutorial must not show STAGE CLEAR");
        for(int area=2;area<=5;++area){
            g.transitionCastleArea(area,CastleTerrain::arrivalX(area));d.onAreaChanged(g);
            check(!d.portalOpen(),"New room starts sealed");
            check(!g.enemies.empty(),"Room has foes");
            if(area<=2)check(d.stage==StoryStage::FirstFoes,"A2 is still the first foes");
            if(area==3||area==4)check(d.stage==StoryStage::Interior,"A3/A4 are the interior");
            if(area==4){bool boss=false;for(auto& e:g.enemies)boss|=e.kind==Executioner&&e.boss;check(boss,"A4 mid-boss missing");}
            if(area==5)check(d.stage==StoryStage::Core,"A5 is the core");
            if(area<5){clearFoes(g);tickStory(d,g,Actions{},2);check(d.portalOpen(),"Exit should open after clear");}
        }
        clearFoes(g);tickStory(d,g,Actions{},2);
        check(d.stage==StoryStage::KingPhase1&&kingOf(g)&&kingOf(g)->kind==DemonKingArmored,"King phase 1 should follow the core wave");
    });
    test("Tutorial roster is weakened and player cannot die before the king's second form",[]{
        StoryDirector d;Combat g;d.start(g);d.requestSkip(g);
        g.player.x=1500;tickStory(d,g,Actions{},2);
        Enemy plain(AshInfantry,0,1);
        check(g.enemies[0].maxHealth<plain.maxHealth,"Tutorial fodder must be weaker than production");
        g.damagePlayer(100000);
        check(g.player.health==1&&!g.defeated&&!g.ended,"Safety floor should hold before phase 2");
    });
    test("Demon King armor breaks into an unbeatable true form",[]{
        StoryDirector d;Combat g;d.start(g);d.requestSkip(g);
        g.transitionCastleArea(5,CastleTerrain::arrivalX(5));d.onAreaChanged(g);
        clearFoes(g);tickStory(d,g,Actions{},2);
        Enemy* k=kingOf(g);check(k!=nullptr,"King missing");
        check(k->wake>0,"King arrives with a slow wake-up");
        k->wake=0;
        const int before=k->health;g.hit(*k,100000,false);
        check(k->kind==DemonKingArmored&&k->health==1&&k->phaseShift>0,"Phase 1 defeat must start the armor break, not death");
        g.hit(*k,500,false);check(k->health==1,"King is untouchable during the transformation");
        check(before>0&&g.foes()>=1,"King still counts as a foe");
        tickStory(d,g,Actions{},200);
        k=kingOf(g);
        check(k->kind==DemonKingTrue&&k->phase2,"Armor should fall away into the true form");
        check(d.stage==StoryStage::KingPhase2&&g.playerHealthFloor==0,"Phase 2 lifts the safety floor");
        g.hit(*k,100000,false);
        near(float(k->health),float(k->maxHealth*35/100));
    });
    test("Boss move selection: six varied one-shots in phase 1, six combos plus an enraged finisher in phase 2",[]{
        std::set<int> one,combo,enraged;BossMove last=BossMove::None;
        for(int i=0;i<24;++i){const BossMove m=bossPickMove(false,i,200,false,last);one.insert(int(m));check(m!=last,"Repeated a phase 1 move");last=m;}
        check(one.size()>=5,"Phase 1 needs at least five distinct moves");
        last=BossMove::None;
        for(int i=0;i<24;++i){const BossMove m=bossPickMove(true,i,200,false,last);combo.insert(int(m));check(m!=last,"Repeated a combo");last=m;}
        check(combo.size()>=5&&!combo.count(int(BossMove::ExecutionCombo)),"Phase 2 varied combos, finisher only when enraged");
        last=BossMove::None;
        for(int i=0;i<24;++i){const BossMove m=bossPickMove(true,i,200,true,last);enraged.insert(int(m));last=m;}
        check(enraged.count(int(BossMove::ExecutionCombo)),"Enraged phase 2 must use the finisher chain");
        for(int i=0;i<12;++i){
            const BossMove far=bossPickMove(true,i,500,false,BossMove::None);
            check(far!=BossMove::DashCombo&&far!=BossMove::SlashBurst&&far!=BossMove::BlinkStrike,"Melee combo picked from far away");
            const BossMove farOne=bossPickMove(false,i,400,false,BossMove::None);
            check(farOne!=BossMove::Smash&&farOne!=BossMove::IronSweep,"Short-reach blow picked from far away");
        }
    });
    test("Phase 1 quake sends ground waves both ways: they hit a grounded player and pass under a jumper",[]{
        auto run=[](bool airborne){
            Combat g=kingArena(DemonKingArmored);Enemy& k=g.enemies.back();
            const float y0=g.player.y-(airborne?150.f:0.f);
            g.bossStartMove(k,BossMove::Quake);
            const int before=g.player.health;size_t waves=0;
            for(int i=0;i<150;++i){
                g.player.invulnerable=0;if(airborne){g.player.y=y0;g.player.vy=0;g.player.grounded=false;}
                g.step(1.f/60,{});waves=std::max(waves,g.hazards.size());
            }
            return std::make_pair(before-g.player.health,int(waves));
        };
        const auto ground=run(false),air=run(true);
        check(ground.second==2,"Quake should create two waves");
        check(ground.first>0,"Wave should hit a grounded player");
        check(air.first==0,"Wave should pass under an airborne player");
    });
    test("Judgement marks three void pillars and only hits a player who stays",[]{
        auto run=[](float playerX){
            Combat g=kingArena(DemonKingArmored);Enemy& k=g.enemies.back();
            g.bossStartMove(k,BossMove::Judgement);
            const float markX=g.player.x;const int before=g.player.health;
            check(g.hazards.size()==3,"Three pillars should be marked immediately");
            for(int i=0;i<150;++i){g.player.invulnerable=0;if(i==10)g.player.x=playerX;g.step(1.f/60,{});}
            (void)markX;check(g.hazardSerial==3,"All three pillars should erupt");
            return before-g.player.health;
        };
        check(run(700)>0,"Standing on the mark should hurt");
        check(run(100)==0,"Leaving the marked ground should avoid every pillar");
    });
    test("Leap slam lands on the player's position at takeoff and blasts the area",[]{
        Combat g=kingArena(DemonKingArmored);Enemy& k=g.enemies.back();
        g.bossStartMove(k,BossMove::LeapSlam);const float target=g.player.x;
        const int before=g.player.health;bool airborne=false;
        for(int i=0;i<150;++i){g.player.invulnerable=0;g.step(1.f/60,{});airborne|=k.y<g.player.y-30;}
        check(airborne,"Boss should leave the ground during the leap");
        check(std::abs(k.x-target)<60,"Boss should land near the marked position");
        check(g.player.health<before,"Landing blast should hurt a player who stays");
    });
    test("Juggernaut rush crosses the room once and leaves a long recovery",[]{
        Combat g=kingArena(DemonKingArmored);Enemy& k=g.enemies.back();
        const float x0=k.x;g.bossStartMove(k,BossMove::JuggernautRush);
        for(int i=0;i<140&&k.attackAge>=0;++i){g.player.invulnerable=0;g.step(1.f/60,{});}
        check(std::abs(k.x-x0)>350,"Rush should cover real distance");
        check(k.attackTimer>2.5f,"Rush should leave a long exposed recovery");
    });
    test("Phase 2 dash combo chains three hits with no idle gap, then a short recovery",[]{
        Combat g=kingArena(DemonKingTrue);Enemy& k=g.enemies.back();
        k.x=900;g.player.x=800;g.bossStartMove(k,BossMove::DashCombo);
        int hits=0,previous=g.player.health;bool gap=false;
        for(int i=0;i<150;++i){   // 0.85 + 0.85 + 0.7 s of combo, then the recovery timer starts
            g.player.invulnerable=0;g.step(1.f/60,{});
            if(g.player.health<previous)++hits;previous=g.player.health;
            if(i<142&&k.attackAge<0)gap=true;
        }
        check(!gap,"Combo steps must follow each other immediately");
        check(hits>=2,"A stationary player should take at least two combo hits");
        check(k.attackTimer>=1.f&&k.attackTimer<=1.6f,"Recovery after a combo is short");
    });
    test("Phase 2 pillar and rain moves erupt three and five tracking pillars; blink lands behind the player",[]{
        {Combat g=kingArena(DemonKingTrue);Enemy& k=g.enemies.back();g.bossStartMove(k,BossMove::PillarChain);
         for(int i=0;i<400&&(k.attackAge>=0||k.move!=BossMove::None);++i){g.player.invulnerable=1;g.step(1.f/60,{});}
         check(g.hazardSerial==3,"PillarChain should erupt three pillars");}
        {Combat g=kingArena(DemonKingTrue);Enemy& k=g.enemies.back();g.bossStartMove(k,BossMove::VoidRain);
         for(int i=0;i<400&&(k.attackAge>=0||k.move!=BossMove::None);++i){g.player.invulnerable=1;g.step(1.f/60,{});}
         check(g.hazardSerial==5,"VoidRain should erupt five pillars");}
        {Combat g=kingArena(DemonKingTrue);Enemy& k=g.enemies.back();k.x=1200;g.player.x=700;
         const unsigned blink=g.kingBlinkSerial;g.bossStartMove(k,BossMove::BlinkStrike);
         check(g.kingBlinkSerial==blink+1,"Blink should flag a teleport");
         check(k.x<g.player.x&&std::abs(k.x-g.player.x)<120,"Blink should land close, on the far side of the player");}
    });
    test("Slash-burst combo ends in a void burst that hits at close range",[]{
        Combat g=kingArena(DemonKingTrue);Enemy& k=g.enemies.back();
        k.x=900;g.player.x=800;const unsigned bursts=g.kingBurstSerial;g.bossStartMove(k,BossMove::SlashBurst);
        for(int i=0;i<220&&g.kingBurstSerial==bursts;++i){g.player.invulnerable=0;g.step(1.f/60,{});}
        check(g.kingBurstSerial>bursts,"The combo never reached its burst");
        check(g.player.health<100,"Burst should hurt a player standing inside the radius");
    });
    test("Every boss step plays a motion its form owns, and the tip table covers its contact frame",[]{
        const BossMove armored[]={BossMove::Smash,BossMove::Quake,BossMove::LeapSlam,BossMove::Judgement,BossMove::IronSweep,BossMove::JuggernautRush};
        const BossMove trueForm[]={BossMove::DashCombo,BossMove::SlashBurst,BossMove::PillarChain,BossMove::BlinkStrike,
            BossMove::WaveVolley,BossMove::VoidRain,BossMove::AerialDive,BossMove::ExecutionCombo};
        std::set<int> used1,used2;
        for(int form=0;form<2;++form){
            const BossMove* list=form?trueForm:armored;const int n=form?8:6;
            for(int i=0;i<n;++i)for(int st=0;st<bossMoveSteps(list[i]);++st){
                const auto d=bossStepDef(list[i],st);
                check(bossMotionOwned(form?DemonKingTrue:DemonKingArmored,d.motion),"Step uses a motion its form does not have");
                (form?used2:used1).insert(int(d.motion));
                const int f=bossContactFrame(d);check(f>=d.animFrom&&f<bossSheetFrames(d.motion),"Contact frame outside the sheet");
                const auto* tip=bossTips[form][int(d.motion)][f];
                check(tip[0]>0&&tip[1]>0,"No sword-tip data for a step's contact frame");
            }
        }
        check(used1.size()>=6,"Armored form should use six different motions");
        check(used2.size()>=8,"True form should use eight different motions");
    });
    test("Boss effects are scheduled as separate layers and land on the contact moment",[]{
        Combat g=kingArena(DemonKingArmored);Enemy& k=g.enemies.back();
        g.bossStartMove(k,BossMove::Smash);
        const BossFxEvent* hammer=nullptr;
        for(const auto& fx:g.bossFx)if(fx.kind==BossFx::Hammer)hammer=&fx;
        check(hammer!=nullptr,"Smash should schedule the hammer impact");
        check(hammer->age<0,"The impact is in the future, not already playing");
        const auto& sheet=bossFxSheets[int(BossFx::Hammer)];
        const float contact=1.6f*.57f;
        near(hammer->age+contact,sheet.peak/sheet.fps);   // impact frame meets contact
        Combat q=kingArena(DemonKingArmored);Enemy& kq=q.enemies.back();
        q.bossStartMove(kq,BossMove::Quake);
        bool spike=false,rune=false;
        for(const auto& fx:q.bossFx){spike|=fx.kind==BossFx::Spike;rune|=fx.kind==BossFx::RuneCircle&&fx.life>0;}
        check(spike,"Quake schedules the stone spike");
        check(rune,"Quake gathers a looping rune circle before the strike");
        for(int i=0;i<140;++i)q.step(1.f/60,{});
        bool spikes=false;for(const auto& h:q.hazards)spikes|=h.fx==BossFx::Spikes;
        check(spikes,"Quake waves are drawn with the ground-spike layer");
    });
    test("Every move stacks several different effect layers, heavy ones at least five",[]{
        struct Row{int kind;BossMove move;int minLayers;};
        const Row rows[]={{DemonKingArmored,BossMove::Smash,5},{DemonKingArmored,BossMove::Quake,6},{DemonKingArmored,BossMove::LeapSlam,6},
            {DemonKingArmored,BossMove::Judgement,6},{DemonKingArmored,BossMove::IronSweep,5},{DemonKingArmored,BossMove::JuggernautRush,6},
            {DemonKingTrue,BossMove::DashCombo,4},{DemonKingTrue,BossMove::SlashBurst,6},{DemonKingTrue,BossMove::PillarChain,5},
            {DemonKingTrue,BossMove::BlinkStrike,5},{DemonKingTrue,BossMove::WaveVolley,3},{DemonKingTrue,BossMove::VoidRain,5},
            {DemonKingTrue,BossMove::AerialDive,6},{DemonKingTrue,BossMove::ExecutionCombo,6}};
        for(const auto& r:rows){
            Combat g=kingArena(r.kind);Enemy& k=g.enemies.back();k.x=900;g.player.x=600;
            g.bossStartMove(k,r.move);
            std::set<int> layers;
            for(int i=0;i<600&&(k.attackAge>=0||k.move!=BossMove::None);++i){
                for(const auto& fx:g.bossFx)layers.insert(int(fx.kind));
                for(const auto& h:g.hazards)layers.insert(int(h.fx));
                g.player.invulnerable=1;g.step(1.f/60,{});
            }
            check(int(layers.size())>=r.minLayers,"A move is too thin: too few effect layers");
        }
    });
    test("Aerial dive: backflip away, leave the ground, crash onto the player's mark, then lunge",[]{
        Combat g=kingArena(DemonKingTrue);Enemy& k=g.enemies.back();
        k.x=900;g.player.x=700;const float x0=k.x;
        g.bossStartMove(k,BossMove::AerialDive);
        check(k.motion==int(BossMotion::Backflip),"Backflip first");
        bool flipped=false,dove=false,crashed=false;float maxAir=0;
        for(int i=0;i<200;++i){
            g.player.invulnerable=1;g.step(1.f/60,{});
            const float ground=CastleTerrain::groundAt(5,k.x);
            maxAir=std::max(maxAir,ground-k.y);
            flipped|=k.x>x0+60;dove|=k.motion==int(BossMotion::Dive);
            for(const auto& fx:g.bossFx)crashed|=fx.kind==BossFx::Dirt;
        }
        check(flipped,"The backflip should carry him away from the player");
        check(dove&&crashed,"The dive and its crash layer should happen");
        check(maxAir>60,"Both the flip and the dive should leave the floor");
    });
    test("Combo steps change body motion so no two consecutive hits look the same",[]{
        for(BossMove m:{BossMove::DashCombo,BossMove::BlinkStrike,BossMove::ExecutionCombo,BossMove::SlashBurst}){
            for(int st=1;st<bossMoveSteps(m);++st)
                check(bossStepDef(m,st).motion!=bossStepDef(m,st-1).motion||bossStepDef(m,st).action==BossAction::Blink,"Consecutive steps repeat a motion");
        }
    });
    test("Boss attack frames can start mid-sheet so short combo hits reuse the same art",[]{
        check(enemyFrame(DemonKingTrue,0,0.f,1.f,4)==4,"Combo hit should start at its first frame");
        check(enemyFrame(DemonKingTrue,0,.999f,1.f,4)==13,"Combo hit should end on the last frame");
        check(enemyFrame(DemonKingTrue,0,.999f,1.f,0,bossMotionFrames)==bossMotionFrames-1,"Motion sheets are 20 frames long");
        check(enemyFrame(DemonKingTrue,0,.5f,1.f,0,bossMotionFrames)==bossMotionFrames/2,"Motion sheet frame follows the step clock");
        // Shaped timing: slow wind-up, impact frame held, quick follow-through, always monotonic.
        {int prev=-1;const float c=.5f;
         for(int i=0;i<=200;++i){
            const int f=enemyFrame(DemonKingTrue,0,i/200.f,1.f,0,bossMotionFrames,c);
            check(f>=prev,"Shaped boss timing must never run backwards");prev=f;}
         const int impact=int(c*bossMotionFrames);
         check(enemyFrame(DemonKingTrue,0,c,1.f,0,bossMotionFrames,c)==impact,"The impact frame must show at contact");
         check(enemyFrame(DemonKingTrue,0,c-.03f,1.f,0,bossMotionFrames,c)==impact,"The impact frame is held just before contact");
         check(enemyFrame(DemonKingTrue,0,c+.03f,1.f,0,bossMotionFrames,c)==impact,"...and just after");
         check(enemyFrame(DemonKingTrue,0,.2f,1.f,0,bossMotionFrames,c)<int(.2f*bossMotionFrames),"The wind-up runs slower than linear");
        }
        check(enemyFrame(DemonKingArmored,0,0.f,1.f)==0,"Full attack starts at frame 0");
    });
    test("Scripted defeat shows no GAME OVER, plays the loss, then wakes in the weakened forest",[]{
        std::remove("story_test.cfg");
        StoryDirector d;d.savePath="story_test.cfg";Combat g;d.start(g);d.requestSkip(g);
        g.transitionCastleArea(5,CastleTerrain::arrivalX(5));d.onAreaChanged(g);
        clearFoes(g);tickStory(d,g,Actions{},2);
        Enemy* k=kingOf(g);k->wake=0;g.hit(*k,100000,false);tickStory(d,g,Actions{},200);
        check(d.stage==StoryStage::KingPhase2,"Need phase 2");
        g.damagePlayer(100000);
        check(g.defeated&&!g.ended,"Lethal damage in phase 2 is a scripted loss, never GAME OVER");
        tickStory(d,g,Actions{},2);
        check(d.stage==StoryStage::Death,"Death sequence should start");
        bool rewound=false,echo=false,glint=false;int maxPips=0,snaps=0;float lastFlash=0;
        for(int i=0;i<60*30&&d.stage==StoryStage::Death;++i){
            tickStory(d,g,Actions{},1);
            rewound|=d.view.rewind;echo|=d.view.desaturate>0;glint|=d.view.memoryGlint>.9f;
            maxPips=std::max(maxPips,d.view.loopPips);
            if(d.view.rewind&&d.view.flash>.85f&&lastFlash<=.85f)++snaps;   // a loop snapping back to the start
            lastFlash=d.view.flash;
        }
        check(rewound&&echo,"Death must show class loss and time reversal");
        check(maxPips==StoryDirector::regressionLoops,"The regression repeats: one tally mark per loop");
        check(snaps>=StoryDirector::regressionLoops-1,"Each loop snaps back and restarts");
        check(glint,"One memory must survive the last regression");
        check(d.stage==StoryStage::Forest,"Sequence should end in the starting forest");
        check(g.castleArea==0&&!g.defeated,"Forest uses the open-air scene");
        check(g.handicap.active&&g.handicap.skills==0&&g.player.handicapped,"Rebirth strips the legendary power");
        check(d.reincarnated,"First reincarnation must persist");
        Combat other;StoryDirector reload;reload.savePath="story_test.cfg";reload.load();
        check(reload.reincarnated,"Story progress should survive a restart");
        reload.start(other);
        check(reload.stage==StoryStage::Forest,"After reincarnation a new game starts in the forest");
        std::remove("story_test.cfg");
    });
    test("Forest returns one power after the first victory; blade baseline cannot swap or skill early",[]{
        StoryDirector d;Combat g;d.reincarnated=true;d.start(g);
        check(d.stage==StoryStage::Forest&&g.castleArea==0,"Forest start");
        check(d.wakeRemaining()>0&&d.inputLocked(),"Protagonist rises slowly before control returns");
        tickStory(d,g,Actions{},200);
        Actions skill;skill.skill=true;skill.swap=true;
        const int jobBefore=g.player.activeJob;
        tickStory(d,g,skill,3);
        check(g.player.activeJob==jobBefore&&g.player.mageSkillCooldown<=0,"Locked skill/swap must do nothing");
        check(g.player.health<g.player.maxHealth(),"Weakened start health");
        check(g.player.moveScale<1.f&&g.player.damageScale<1.f,"Weak tentative movement and damage");
        for(auto& e:g.enemies){e.health=0;break;}
        tickStory(d,g,Actions{},2);
        check(g.handicap.skills==1&&d.view.highlight==ControlLayout::Skill,"First kill should reclaim one skill with a visual cue");
    });
    test("Phase 2 always ends: failsafe timer forces the scripted defeat even when never hit",[]{
        StoryDirector d;Combat g;d.start(g);d.requestSkip(g);
        g.transitionCastleArea(5,CastleTerrain::arrivalX(5));d.onAreaChanged(g);
        clearFoes(g);tickStory(d,g,Actions{},2);
        Enemy* k=kingOf(g);k->wake=0;g.hit(*k,100000,false);tickStory(d,g,Actions{},200);
        for(int i=0;i<60*45&&d.stage==StoryStage::KingPhase2;++i){
            g.player.health=g.player.maxHealth();g.player.invulnerable=1;   // never gets hit
            for(auto& e:g.enemies)e.attackTimer=1000;                       // king stays passive
            tickStory(d,g,Actions{},1);
        }
        check(d.stage!=StoryStage::KingPhase2,"Phase 2 must not softlock");
    });
    test("Restart key cannot revive or skip the scripted death",[]{
        StoryDirector d;Combat g;d.start(g);d.requestSkip(g);
        g.transitionCastleArea(5,CastleTerrain::arrivalX(5));d.onAreaChanged(g);
        clearFoes(g);tickStory(d,g,Actions{},2);
        Enemy* k=kingOf(g);k->wake=0;g.hit(*k,100000,false);tickStory(d,g,Actions{},200);
        check(d.stage==StoryStage::KingPhase2,"Phase 2 expected before the scripted defeat");
        g.defeatPlayer();tickStory(d,g,Actions{},2);
        check(d.stage==StoryStage::Death,"Lethal damage must start the death sequence");
        Actions r;r.restart=true;tickStory(d,g,r,1);
        check(d.stage==StoryStage::Death&&g.defeated&&g.player.health==0,"Restart must not revive the player or leave the death");
    });
    test("Restarting the forest keeps the weakened baseline",[]{
        StoryDirector d;Combat g;d.reincarnated=true;d.start(g);
        tickStory(d,g,Actions{},200);
        check(!g.enemies.empty(),"Forest foes missing");
        const int weakened=g.enemies[0].maxHealth;
        Actions r;r.restart=true;tickStory(d,g,r,1);
        check(!g.enemies.empty()&&g.enemies[0].maxHealth==weakened,"Restart must re-apply the weakened forest foes");
        check(g.player.health==45,"Restart must keep the 45 HP forest start");
    });
    test("Finishing practice returns the player to the start so the threshold beat is played",[]{
        StoryDirector d;Combat g;d.start(g);g.player.x=1700;d.requestSkip(g);
        near(g.player.x,CastleTerrain::profile(1).spawnX);
        tickStory(d,g,Actions{},2);
        check(d.stage==StoryStage::Threshold,"A runner must not skip the threshold into combat");
    });
    test("Tutorial gatekeeper knight is eased but stays tougher than a soldier",[]{
        StoryDirector d;Combat g;d.start(g);d.requestSkip(g);
        g.transitionCastleArea(3,CastleTerrain::arrivalX(3));d.onAreaChanged(g);
        int knight=0,soldier=0;
        for(auto& e:g.enemies){if(e.kind==GatekeeperKnight)knight=e.maxHealth;if(e.kind==AshInfantry)soldier=e.maxHealth;}
        check(knight>0&&soldier>0,"A3 roster should contain a knight and a soldier");
        check(knight>soldier&&knight<soldier*2,"Knight health should sit between 1x and 2x a soldier's");
    });
    test("Space Ruler coefficients: one singularity clears every tutorial soldier; bolt stays below Flash basic dps",[]{
        StoryDirector d;Combat g;d.start(g);d.requestSkip(g);
        for(int area=1;area<=5;++area){
            g.transitionCastleArea(area,CastleTerrain::arrivalX(area));d.onAreaChanged(g);
            for(auto& e:g.enemies)if(e.kind==AshInfantry)check(e.maxHealth<=spaceSingularityDamage,"Singularity must one-shot a tutorial soldier");
        }
        check(spaceBoltDamage/.55f<15/.46f,"Void bolt single-target dps should stay under the Flash basic dps");
        check(spaceBoltDamage>11&&spaceSpikesDamage>24&&spaceSingularityDamage>65,"Space Ruler coefficients were raised");
    });
    test("Progression: default stats change nothing; cost curve and caps hold",[]{
        const StatBlock none=Progression{}.blockFor(1);
        check(none.damageMul==1.f&&none.bonusHealth==0&&none.bonusDefense==0&&none.critChance==0.f&&none.moveMul==1.f&&none.cooldownRate==1.f,"No levels must mean no bonus");
        check(Progression::applyDamage(37,none,0.f)==37,"No stats must leave damage untouched, even on roll 0");
        check(Progression::upgradeCost(0)==10&&Progression::upgradeCost(1)==13,"Cost curve start");
        check(Progression::upgradeCost(maxStatLevel)==0,"Maxed stat costs nothing more");
        for(int l=0;l<maxStatLevel;++l)check(Progression::upgradeCost(l+1)>=Progression::upgradeCost(l),"Cost must not fall");
        StatLevels max;max.fill(maxStatLevel);
        const StatBlock top=Progression::block(max);
        near(top.damageMul,1.5f);check(top.bonusHealth==100&&top.bonusDefense==10,"Max vitality/guard");
        near(top.critChance,.30f);near(top.critMul,2.3f);near(top.moveMul,1.15f);near(top.cooldownRate,1.2f);
        StatLevels over;over.fill(999);
        check(Progression::block(over).bonusHealth==100,"Levels beyond the cap are clamped");
    });
    test("Progression: upgrades eat memories per job and crits are deterministic",[]{
        Progression p;p.memories=25;
        check(p.upgrade(1,Stat::Attack)&&p.memories==15&&p.level(1,Stat::Attack)==1,"First upgrade costs 10");
        check(p.upgrade(1,Stat::Attack)&&p.memories==2,"Second costs 13");
        check(!p.upgrade(1,Stat::Attack)&&p.level(1,Stat::Attack)==2&&p.memories==2,"Too expensive: nothing changes");
        check(p.level(2,Stat::Attack)==0&&p.level(3,Stat::Attack)==0,"Other jobs are untouched");
        check(!p.canUpgrade(0,Stat::Attack)&&!p.canUpgrade(9,Stat::Attack),"Unknown job cannot upgrade");
        StatBlock crit;crit.critChance=.3f;crit.critMul=2.f;
        bool isCrit=false;
        check(Progression::applyDamage(100,crit,.29f,&isCrit)==200&&isCrit,"Roll under the chance crits");
        check(Progression::applyDamage(100,crit,.30f,&isCrit)==100&&!isCrit,"Roll at the chance does not crit");
        check(Progression::memoryDrop(10,false)==1&&Progression::memoryDrop(400,false)==10&&Progression::memoryDrop(400,true)==50,"Memory drops scale with health; bosses x5");
    });
    test("Progression: save file round-trips and a damaged file is ignored",[]{
        const std::string path="progress_test.cfg";
        Progression a;a.memories=321;a.levels[0][2]=7;a.levels[1][6]=20;a.levels[2][0]=3;
        check(a.save(path),"Save failed");
        Progression b;b.memories=5;check(b.load(path),"Load failed");
        check(b.memories==321&&b.levels[0][2]==7&&b.levels[1][6]==20&&b.levels[2][0]==3,"Round trip mismatch");
        {std::FILE* f=std::fopen(path.c_str(),"w");std::fputs("ECLIPSE_PROGRESS 1 memories 9\njob 1 1 2 3\n",f);std::fclose(f);}
        Progression c;c.memories=77;check(!c.load(path)&&c.memories==77,"A truncated file must leave the state untouched");
        {std::FILE* f=std::fopen(path.c_str(),"w");std::fputs("ECLIPSE_PROGRESS 1 memories 4\njob 1 99 -5 0 0 0 0 0\njob 2 0 0 0 0 0 0 0\njob 3 0 0 0 0 0 0 0\n",f);std::fclose(f);}
        Progression d;check(d.load(path)&&d.levels[0][0]==maxStatLevel&&d.levels[0][1]==0,"Out-of-range levels are clamped");
        std::remove(path.c_str());
        Progression missing;check(!missing.load("no_such_progress_file.cfg"),"Missing file is not an error state");
    });
    test("Stats reach combat: attack, vitality, guard, swiftness and haste",[]{
        Combat g;const int j=Progression::jobIndex(g.activeJob());
        const int baseHealth=g.player.maxHealth();const float baseMove=g.player.movementMultiplier();
        g.progression.levels[j][static_cast<int>(Stat::Attack)]=20;
        g.progression.levels[j][static_cast<int>(Stat::Vitality)]=20;
        g.progression.levels[j][static_cast<int>(Stat::Guard)]=20;
        g.progression.levels[j][static_cast<int>(Stat::Swiftness)]=20;
        g.progression.levels[j][static_cast<int>(Stat::Haste)]=20;
        g.refreshStats();
        check(g.player.maxHealth()==baseHealth+100,"Vitality adds 100 max health");
        near(g.player.movementMultiplier(),baseMove*1.15f);
        Enemy& e=g.enemies[0];e.health=e.maxHealth=100000;e.wake=0;
        g.hit(e,100,false);
        check(100000-e.health==150,"Attack 20 is +50 % on a direct hit");
        g.player.invulnerable=0;g.player.health=g.player.maxHealth();
        g.damagePlayer(30);
        check(g.player.health==g.player.maxHealth()-20,"Guard 20 absorbs 10 flat");
        g.player.mageSkillCooldown=7;g.player.buffReady=7;g.player.dashCooldown=7;
        wait(g,60);
        check(std::abs(g.player.mageSkillCooldown-5.8f)<.05f&&std::abs(g.player.buffReady-5.8f)<.05f,"Haste speeds skill cooldowns by 20 %");
        check(g.player.dashCooldown>5.95f,"Haste must not speed up the dash (7 s - 1 s, not 1.2 s)");
    });
    test("Stats follow the active job and raising max health keeps current health",[]{
        Combat g;g.player.equippedJobs={{1,2}};g.restart();
        g.progression.levels[1][static_cast<int>(Stat::Vitality)]=10;
        g.refreshStats();
        check(g.player.statHealth==0,"Job 1 has no vitality levels");
        g.player.health=40;
        Actions swap;swap.swap=true;g.step(1.f/60,swap);
        check(g.activeJob()==2&&g.player.statHealth==50,"Swapping to job 2 applies its own levels");
        check(g.player.health==40+50,"Gaining max health also raises current health by the same amount");
        g.progression.memories=1000;
        check(g.upgradeStat(Stat::Vitality)&&g.player.statHealth==55,"upgradeStat refreshes the bonus immediately");
    });
    test("Memories drop on kills only when enabled, once per enemy, 5x for bosses",[]{
        Combat g;
        Enemy& a=g.enemies[0];a.health=a.maxHealth=400;a.wake=0;
        g.hit(a,100000,false);
        check(g.progression.memories==0,"Drops are off by default");
        a.rewarded=false;
        Combat h;h.dropsEnabled=true;
        Enemy& e=h.enemies[0];e.health=e.maxHealth=400;e.wake=0;
        h.hit(e,100000,false);
        check(h.progression.memories==10,"400 max health drops 10 memories");
        e.health=5;h.hit(e,100000,false);   // revived and killed again: the paid flag stays set
        check(h.progression.memories==10,"An enemy never pays twice");
        Enemy& b=h.enemies[1];b.health=b.maxHealth=400;b.boss=true;b.wake=0;
        h.hit(b,100000,false);
        check(h.progression.memories==60,"A boss drops five times as many");
    });
    test("Story: no shard drops in the prologue, drops from the starting forest on",[]{
        StoryDirector d;Combat g;d.start(g);
        check(!g.dropsEnabled,"Prologue has no shard economy");
        StoryDirector r;Combat h;r.reincarnated=true;r.start(h);
        check(h.dropsEnabled,"Starting forest enables shard drops");
        check(h.player.equippedJobs[0]==3&&h.stats.bonusHealth==0,"Blade baseline starts with its own (empty) stats");
    });
    test("Progression: save files written before the memory rename still load",[]{
        const std::string path="progress_legacy_test.cfg";
        {std::FILE* f=std::fopen(path.c_str(),"w");std::fputs("ECLIPSE_PROGRESS 1 shards 42\njob 1 1 0 0 0 0 0 0\njob 2 0 2 0 0 0 0 0\njob 3 0 0 3 0 0 0 0\n",f);std::fclose(f);}
        Progression p;check(p.load(path)&&p.memories==42&&p.levels[0][0]==1&&p.levels[1][1]==2&&p.levels[2][2]==3,"Legacy header must load");
        std::remove(path.c_str());
    });
    test("Memory altar: opens next to the awakening spot, freezes the world, closes again",[]{
        Combat g;wait(g,10);
        check(g.castleArea==0&&g.nearMemoryAltar(),"The forest start is next to the altar");
        const float enemyX=g.enemies[0].x,playerX=g.player.x;
        Actions use;use.interact=true;
        g.step(1.f/60,use);
        check(g.memoryMenu,"Interact next to the altar opens the menu");
        Actions walk;walk.right=true;walk.attack=true;
        wait(g,5);for(int i=0;i<30;++i)g.step(1.f/60,walk);
        check(g.memoryMenu&&g.enemies[0].x==enemyX&&std::abs(g.player.x-playerX)<.01f,"The world is frozen while the menu is open");
        g.step(1.f/60,use);
        check(!g.memoryMenu,"Interact closes the menu");
    });
    test("Memory altar: only at the forest start, never in the castle, and a restart closes it",[]{
        Combat g;wait(g,10);
        g.player.x=800;wait(g,2);
        check(!g.nearMemoryAltar(),"Far from the start there is no altar");
        Actions use;use.interact=true;g.step(1.f/60,use);
        check(!g.memoryMenu,"Interact far from the altar does nothing");
        Combat castle;castle.configureCastleArea(1);castle.player.x=memoryAltarX;
        check(!castle.nearMemoryAltar(),"The castle has no altar");
        Combat h;wait(h,10);h.step(1.f/60,use);check(h.memoryMenu,"Menu opens");
        h.restart();check(!h.memoryMenu,"Restart closes the menu");
    });
    test("Eating memories pulses; fallen enemies send a memory to the protagonist only when drops are on",[]{
        Combat g;g.dropsEnabled=true;
        Enemy& e=g.enemies[0];e.health=e.maxHealth=400;e.wake=0;
        g.hit(e,100000,false);
        check(g.memoryMotes.size()==1&&g.memoryMotes[0].amount==10,"A kill sends one memory mote worth 10");
        wait(g,60);
        check(g.memoryMotes.empty(),"The mote is gone after it reached the protagonist");
        g.progression.memories=100;
        check(g.upgradeStat(Stat::Attack)&&g.memoryFlash>0.f,"Eating memories pulses");
        Combat quiet;Enemy& q=quiet.enemies[0];q.health=q.maxHealth=400;q.wake=0;quiet.hit(q,100000,false);
        check(quiet.memoryMotes.empty(),"No mote when drops are off (prologue)");
    });
    test("Regression loops get faster, then the last one is slow; the whole rewind is bounded",[]{
        const auto& r=StoryDirector::loopRoomSeconds;
        for(int i=0;i<StoryDirector::regressionLoops-2;++i)check(r[i+1]<r[i],"Loops 1-4 must speed up");
        check(r[StoryDirector::regressionLoops-1]>r[StoryDirector::regressionLoops-2],"The last loop slows down again");
        check(StoryDirector::rewindSeconds()>8.f&&StoryDirector::rewindSeconds()<16.f,"Rewind length stays watchable");
    });
    test("Forest memory tutorial: a surviving memory is eaten at the altar, then the next foe's memory is hinted",[]{
        StoryDirector d;Combat g;d.reincarnated=true;d.start(g);
        check(!d.memoryTutorialDone,"The first reincarnation starts the memory tutorial");
        tickStory(d,g,Actions{},200);
        check(g.progression.memories==Progression::upgradeCost(0),"One memory remains from the earlier regressions");
        check(d.view.altarGlow>0.f&&d.view.highlight==ControlLayout::Attack,"The altar and the interact button are pointed out");
        check(d.view.hintX<0.f,"No foe hint before the memory is eaten");
        Actions use;use.interact=true;tickStory(d,g,use,1);
        check(g.memoryMenu&&d.view.menuRow==0,"The altar menu opens with the attack row pointed out");
        check(g.upgradeStat(Stat::Attack),"The remembered memory can be eaten");
        tickStory(d,g,Actions{},70);
        check(!g.memoryMenu,"The menu closes by itself after eating");
        check(d.view.hintX>=0.f&&d.view.altarGlow==0.f,"Then the way to the next foe's memory is shown");
        check(d.stage==StoryStage::Forest,"The tutorial is not over yet");
        for(auto& e:g.enemies){e.health=0;break;}
        tickStory(d,g,Actions{},2);
        check(g.handicap.skills==1,"The first kill also returns a skill");
        tickStory(d,g,Actions{},200);
        check(d.stage==StoryStage::Free&&d.memoryTutorialDone,"The tutorial ends once a memory was eaten and a foe was beaten");
    });
    test("Forest memory tutorial cannot end before the first memory is eaten; it is saved and skipped later",[]{
        const std::string path="story_memory_test.cfg";std::remove(path.c_str());
        StoryDirector d;d.savePath=path;Combat g;d.reincarnated=true;d.start(g);
        tickStory(d,g,Actions{},200);
        for(auto& e:g.enemies){e.health=0;break;}
        tickStory(d,g,Actions{},300);
        check(d.stage==StoryStage::Forest&&!d.memoryTutorialDone,"Killing a foe alone must not finish the tutorial");
        check(g.upgradeStat(Stat::Attack),"Eating the memory afterwards works");
        tickStory(d,g,Actions{},10);
        check(d.stage==StoryStage::Free&&d.memoryTutorialDone,"Then the tutorial ends");
        StoryDirector reload;reload.savePath=path;reload.load();
        check(reload.reincarnated&&reload.memoryTutorialDone,"Both flags are saved");
        Combat other;reload.start(other);
        check(reload.stage==StoryStage::Forest&&other.progression.memories==0,"A later new game does not repeat the gift");
        tickStory(reload,other,Actions{},200);
        check(other.progression.memories==0&&reload.view.altarGlow==0.f,"No tutorial cues once it was completed");
        {std::FILE* f=std::fopen(path.c_str(),"w");std::fputs("ECLIPSE_STORY 1 reincarnated 1\n",f);std::fclose(f);}
        StoryDirector old;old.savePath=path;old.load();
        check(old.reincarnated&&old.memoryTutorialDone,"A save from before the tutorial counts as already played");
        std::remove(path.c_str());
    });
    test("Mastery tree: parents and need decide what opens (pyramid core plus extra nodes)",[]{
        check(MasteryState::earnedPoints(0)==0&&MasteryState::earnedPoints(2)==0&&MasteryState::earnedPoints(3)==1,"First point at 3 xp");
        check(MasteryState::earnedPoints(8)==2&&MasteryState::earnedPoints(15)==3&&MasteryState::earnedPoints(24)==4,"Points at 8, 15, 24 xp");
        check(masteryNodeCount(1)==13&&masteryNodeCount(2)==13&&masteryNodeCount(3)==13&&masteryNodeCount(0)==0,"13 nodes per job, none for a bad job");
        check(MasteryState::nodeCost(1,0)==1&&MasteryState::nodeCost(1,3)==1&&MasteryState::nodeCost(1,4)==2&&MasteryState::nodeCost(1,6)==2,"Tier 1 costs 1, tier 2 costs 2");
        check(MasteryState::nodeCost(1,7)==3&&MasteryState::nodeCost(1,9)==5&&MasteryState::nodeCost(1,10)==3&&MasteryState::nodeCost(1,12)==6&&MasteryState::totalCost(1)==33,"Costs add up to 33");
        MasteryState m;
        check(!m.buy(1,0)&&m.points(1)==0,"No points, no node");
        m.addXp(1,8);                                       // 2 points
        check(m.points(1)==2&&m.points(2)==0&&m.points(3)==0,"xp belongs to the job that earned it");
        check(m.buy(1,0)&&m.points(1)==1,"Tier 1 nodes are always open");
        check(!m.unlocked(1,4),"One parent is not enough when two are needed");
        check(m.buy(1,1)&&m.points(1)==0,"Second parent");
        check(m.unlocked(1,4)&&!m.canBuy(1,4),"Both parents own it open, but there are no points left yet");
        check(!m.unlocked(1,5),"Node 5 needs nodes 1 and 2: node 2 is missing");
        m.addXp(1,16);                                      // 24 xp: 4 points, 2 spent
        check(m.canBuy(1,4)&&m.buy(1,4)&&m.points(1)==0,"Tier 2 costs 2 points");
        check(!m.buy(1,4),"A node is bought once");
        MasteryState t;t.addXp(1,5000);
        for(int n=0;n<4;++n)check(t.buy(1,n),"Open tier 1");
        check(t.buy(1,4)&&!t.unlocked(1,7),"Needs two of three tier-2 nodes: one is not enough");
        check(t.buy(1,5)&&t.unlocked(1,7)&&t.unlocked(1,8),"Any two of the three tier-2 nodes open both");
        check(t.buy(1,7)&&!t.unlocked(1,9),"The top of the pyramid needs BOTH tier-3 nodes");
        check(t.unlocked(1,11)&&!t.unlocked(1,10),"Node 11 needs only ONE of its parents; node 10 needs 5 AND 6");
        check(t.buy(1,11),"A node with need 1 opens on one parent");
        check(t.buy(1,8)&&t.buy(1,9),"Both tier-3 nodes open the top");
        check(!t.unlocked(1,12),"The extra capstone needs nodes 9 and 10");
        check(t.buy(1,6)&&t.buy(1,10)&&t.unlocked(1,12)&&t.buy(1,12),"With 9 and 10 it opens");
        check(!t.has(2,0)&&!t.canBuy(0,0)&&!t.canBuy(9,0)&&!t.canBuy(1,13)&&!t.canBuy(1,-1),"Other jobs and bad indices are untouched");
        check(std::string(masteryNodeName(1,0))=="DENSE CORE"&&std::string(masteryNodeTag(2,5))=="LANCES +2","Names and tags exist for the UI");
        check(masteryNodeIsRaw(2,0)&&!masteryNodeIsRaw(2,1)&&masteryNodeIsRaw(3,8)&&!masteryNodeIsRaw(1,10),"Raw and signature/special nodes are told apart");
    });
    test("Mastery data stays valid however large the trees grow",[]{
        for(int job=1;job<=3;++job){
            const auto& tree=masteryTree(job);
            check(static_cast<int>(tree.size())<=maxMasteryNodes,"The owned-flag array is big enough");
            for(int i=0;i<static_cast<int>(tree.size());++i){
                const auto& n=tree[i];
                check(n.parentCount>=0&&n.parentCount<=maxNodeParents&&n.need>=0&&n.need<=n.parentCount,"need never exceeds the parents");
                for(int k=0;k<n.parentCount;++k)check(n.parents[k]>=0&&n.parents[k]<i,"Parents exist and come earlier (no cycles)");
                check((n.parentCount==0)==(n.need==0),"A node with parents needs at least one");
                check(n.cost>=1&&n.modCount+n.procCount>0,"Every node costs something and does something");
                check(std::string(n.name).size()>0&&std::string(n.name).size()<=11,"Name fits the node box");
                check(std::string(n.tag).size()<=16&&std::string(n.desc).size()<=44,"Tag and description fit");
                for(int k=0;k<n.procCount;++k)check(n.procs[k].chance>0.f&&n.procs[k].chance<=1.f&&n.procs[k].every>=1,"Procs have a sane chance and cadence");
                for(int k=0;k<i;++k)check(!(tree[k].x==n.x&&tree[k].y==n.y),"No two nodes share a spot");
            }
        }
    });
    test("Mastery blocks: no nodes means no change; each job's nodes change only that job",[]{
        const MasteryBlock none=MasteryState{}.block(1);
        check(none.boltMul==1.f&&!none.twinBolt&&none.comboMul==1.f&&none.rawDefense==0&&none.rawDamage==0.f&&none.rawHealth==0&&none.lanceCapBonus==0&&none.procs.empty(),"Defaults are neutral");
        MasteryState m;m.xp={{5000,5000,5000}};
        for(int j=1;j<=3;++j)for(int n=0;n<masteryNodeCount(j);++n)check(m.buy(j,n),"Buying in index order always follows the tree rules");
        const MasteryBlock space=m.block(1),flash=m.block(2),blade=m.block(3);
        check(space.twinBolt&&space.boltMul==1.25f&&space.repulseRadiusMul==1.25f&&space.singularityMul==1.5f&&space.rawHealth==30&&space.rawHaste>0.f,"Space Ruler tree");
        check(flash.lanceCapBonus==2&&flash.rawHaste>0.f&&flash.rawMove>0.f&&flash.rawDamage>0.f&&flash.rushMul==1.4f&&flash.blinkInvulnerability>0.f,"Flash Executioner tree");
        check(blade.comboMul==1.25f&&blade.bleedStacksBonus==1&&blade.rawDefense==5&&blade.rawHealth==40&&blade.healPerKill==3&&blade.lastStandMul==1.3f,"Blade tree (defense 2 + 3)");
        check(space.rawDamage==0.f&&flash.boltMul==1.f&&blade.boltMul==1.f&&blade.lanceCapBonus==0&&flash.rawDefense==0,"A job's nodes never leak into another job");
        check(space.procs.size()==3&&flash.procs.size()==4&&blade.procs.size()==4,"Special effects of the owned nodes are collected");
    });
    test("Mastery in combat: kills feed the active job, nodes change its numbers, the tree resets on death",[]{
        Combat g;g.dropsEnabled=true;
        const int job=g.activeJob();
        Enemy& e=g.enemies[0];e.health=e.maxHealth=400;e.wake=0;
        g.hit(e,100000,false);
        check(g.mastery.xpOf(job)==1,"A kill gives the active job one xp");
        Enemy& b=g.enemies[1];b.health=b.maxHealth=400;b.boss=true;b.wake=0;
        g.hit(b,100000,false);
        check(g.mastery.xpOf(job)==6,"A boss gives five xp");
        for(int other=1;other<=3;++other)if(other!=job)check(g.mastery.xpOf(other)==0,"Other jobs gain nothing");
        check(g.mastery.points(job)==MasteryState::earnedPoints(6),"xp became points");
        g.mastery.xp[Progression::jobIndex(job)]=100;g.refreshStats();
        check(g.buyMasteryNode(0)&&g.perks.boltMul==1.25f,"Buying a node refreshes the active job's numbers");
        check(g.memoryFlash>0.f,"Buying pulses like eating a memory");
        g.restart();
        check(g.mastery.has(job,0),"A plain restart keeps the run");
        g.player.invulnerable=0;g.damagePlayer(100000);
        check(g.ended&&!g.won&&g.runLost,"Death ends the run");
        g.restart();
        check(!g.mastery.has(job,0)&&g.mastery.xpOf(job)==0&&g.perks.boltMul==1.f&&!g.runLost,"The next run starts with an empty tree");
        StoryDirector d;Combat h;h.mastery.xp={{50,50,50}};d.reincarnated=true;d.start(h);
        check(h.mastery.xpOf(1)==0,"A new game starts a fresh run");
    });
    test("Mastery effects on the blade: signature and raw nodes change real numbers",[]{
        Combat g;g.player.equippedJobs={{3,3}};g.restart();
        check(g.activeJob()==3&&g.player.form==0,"Blade job");
        g.mastery.xp[Progression::jobIndex(3)]=1000;
        for(int n:{0,1,2,3,4,6,7,8,9})check(g.mastery.buy(3,n),"Nodes are bought in tree order");
        g.refreshStats();
        check(g.perks.comboMul==1.25f&&g.perks.bleedStacksBonus==1&&g.perks.healPerKill==3&&g.perks.lastStandMul==1.3f,"Blade signature numbers");
        check(g.stats.bonusDefense==5&&std::abs(g.stats.damageMul-1.1f)<.001f,"Raw nodes joined the stats: defense +2 +3, damage +10 %");
        Actions swing;swing.attack=true;g.step(1.f/60,swing);
        check(g.player.combo==1&&g.player.meleeDamage==15,"Heavy Hand: first combo hit 12 -> 15");
        g.player.invulnerable=0;g.player.health=g.player.maxHealth();
        g.damagePlayer(30);
        check(g.player.health==g.player.maxHealth()-25,"Thick Skin + Bulwark: 30 damage becomes 25");
        Enemy& e=g.enemies[0];e.health=e.maxHealth=100000;e.wake=0;
        g.player.health=g.player.maxHealth();
        g.hit(e,100,false);check(100000-e.health==110,"Brute: +10 % on a direct hit; Last Stand off at full health");
        g.player.health=30;e.health=100000;
        g.hit(e,100,false);check(100000-e.health==143,"Below 40 % health Last Stand adds 30 % on top (110 -> 143)");
        g.dropsEnabled=true;g.player.health=50;
        Enemy& f=g.enemies[1];f.health=f.maxHealth=40;f.wake=0;g.hit(f,100000,false);
        check(g.player.health==53,"Second Wind heals 3 HP per kill");
    });
    test("Raw job stats join the permanent stats only for their job and reset with the run",[]{
        Combat g;                                                       // jobs {1, 2}: Space Ruler first
        g.progression.levels[0][static_cast<int>(Stat::Vitality)]=10;   // permanent +50 HP for job 1
        g.mastery.xp[0]=1000;g.mastery.xp[1]=1000;
        for(int n:{1,2,3,6})check(g.mastery.buy(1,n),"Space Ruler nodes");   // CLEAR MIND crit, DEEP WELL, LIGHT STEP move, DARK BODY hp
        g.refreshStats();
        check(g.player.maxHealth()==100+50+30,"Permanent +50 and raw +30 add up");
        near(g.stats.critChance,.06f);near(g.player.statMove,1.10f);
        Actions swap;swap.swap=true;g.step(1.f/60,swap);
        check(g.activeJob()==2&&g.player.maxHealth()==100&&g.stats.critChance==0.f&&g.player.statMove==1.f,"Swapping drops the other job's raw stats");
        for(int n:{0,1,2,3,4})check(g.mastery.buy(2,n),"Flash nodes");       // KEEN EDGE, ORB SPRINT, SHARP LANCE, FLEET move, FLOW haste
        g.refreshStats();
        near(g.stats.cooldownRate,1.10f);near(g.player.statMove,1.12f);near(g.stats.damageMul,1.10f);
        check(g.perks.lanceMul==1.3f,"Signature numbers sit next to the raw ones");
        g.player.invulnerable=0;g.damagePlayer(100000);g.restart();
        check(g.stats.cooldownRate==1.f&&g.player.statMove==1.f&&g.stats.critChance==0.f&&g.player.maxHealth()==150,"A lost run removes the raw stats; the permanent +50 HP of job 1 stays");
    });
    test("Altar menu tabs: left/right switch, a row picks from the current tab",[]{
        Combat g;wait(g,10);
        Actions use;use.interact=true;g.step(1.f/60,use);
        check(g.memoryMenu&&g.memoryTab==0,"The menu opens on the permanent tab");
        g.progression.memories=100;g.mastery.xp[Progression::jobIndex(g.activeJob())]=8;g.refreshStats();
        Actions right;right.right=true;g.step(1.f/60,right);
        check(g.memoryTab==1,"Right switches to the skill tree");
        g.step(1.f/60,right);check(g.memoryTab==1,"Holding the key does not flip it back");
        check(g.menuChoose(0)&&g.mastery.has(g.activeJob(),0),"A node on the tree tab costs mastery points");
        check(!g.menuChoose(1)||g.mastery.has(g.activeJob(),1),"Tier 2 is only bought when the points and the tier above allow it");
        check(g.progression.level(g.activeJob(),Stat::Attack)==0&&g.progression.memories==100,"The tree never touches permanent stats");
        Actions left;left.left=true;g.step(1.f/60,left);
        check(g.memoryTab==0&&g.menuChoose(0)&&g.progression.level(g.activeJob(),Stat::Attack)==1,"Left returns to the permanent tab");
        g.memoryTab=1;check(!g.menuChoose(masteryNodeCount(g.activeJob()))&&!g.menuChoose(-1),"Out-of-range nodes do nothing");
    });
    test("Special effects: kill blast, crit bleed, hurt pulse with cooldown, every-Nth refund, lances, heal, chance",[]{
        // Kill blast: the explosion reaches nearby foes once and never chains into another blast.
        Combat g;auto& pr=g.perks.procs;
        pr.push_back(masteryProc(Trigger::OnKill,ProcAction::Explode,100,1.f,500));
        g.enemies[0].health=g.enemies[0].maxHealth=1;g.enemies[1].health=g.enemies[1].maxHealth=10;
        g.enemies[2].health=g.enemies[2].maxHealth=1000;g.enemies[3].health=g.enemies[3].maxHealth=1000;
        for(auto& e:g.enemies)e.wake=0;
        g.hit(g.enemies[0],100000,false);
        check(g.enemies[1].dead(),"The blast killed the foe next to it");
        check(g.enemies[2].health==900,"A foe 445 units away takes exactly one blast (no chain reaction)");
        check(g.enemies[3].health==1000,"A foe 710 units away is out of range");
        check(!g.procCues.empty(),"The blast leaves a visual cue");
        // Crit bleed
        Combat c;c.perks.procs.push_back(masteryProc(Trigger::OnCrit,ProcAction::Bleed,2));
        c.stats.critChance=1.f;Enemy& v=c.enemies[0];v.health=v.maxHealth=100000;v.wake=0;
        c.hit(v,100,false);
        check(v.bleedStacks==2,"A critical hit applies 2 bleed stacks");
        Combat n;n.perks.procs.push_back(masteryProc(Trigger::OnCrit,ProcAction::Bleed,2));
        Enemy& w=n.enemies[0];w.health=w.maxHealth=100000;w.wake=0;n.hit(w,100,false);
        check(w.bleedStacks==0,"A normal hit does not");
        // Hurt pulse with a cooldown
        Combat h;h.perks.procs.push_back(masteryProc(Trigger::OnDamaged,ProcAction::Pulse,20,1.f,2000,8.f));
        Enemy& f=h.enemies[0];f.health=f.maxHealth=1000;f.wake=0;
        h.player.invulnerable=0;h.damagePlayer(10);
        check(f.health==980&&f.knockback>0.f,"Being hurt pushes foes away and damages them");
        h.player.invulnerable=0;h.damagePlayer(10);
        check(f.health==980,"The pulse is on cooldown for 8 seconds");
        wait(h,5);
        check(h.procCooldown[0]<8.f&&h.procCooldown[0]>0.f,"The cooldown counts down with the simulation");
        h.procCooldown[0]=0.f;
        h.player.invulnerable=0;h.damagePlayer(10);
        check(f.health==960,"Once the cooldown is over it fires again");
        // Every Nth trigger
        Combat e;e.perks.procs.push_back(masteryProc(Trigger::OnSkill,ProcAction::RefundCooldown,2.f,1.f,0,0,3));
        e.player.skillCooldown=10;
        e.fireProcs(Trigger::OnSkill,nullptr);e.fireProcs(Trigger::OnSkill,nullptr);
        check(e.player.skillCooldown==10.f,"Not yet: it fires on every 3rd skill");
        e.fireProcs(Trigger::OnSkill,nullptr);
        check(std::abs(e.player.skillCooldown-8.f)<.001f,"The 3rd skill refunds 2 seconds");
        // Lances respect the cap; heal; zero chance never fires
        Combat l;l.player.flashExecutioner=true;l.perks.procs.push_back(masteryProc(Trigger::OnCrit,ProcAction::AddLance,3));
        l.fireProcs(Trigger::OnCrit,nullptr);check(l.player.storedLightLances==3,"A proc stores lances");
        l.fireProcs(Trigger::OnCrit,nullptr);check(l.player.storedLightLances==5,"Never above the cap of 5");
        Combat y;y.perks.procs.push_back(masteryProc(Trigger::OnDash,ProcAction::Heal,7));
        y.player.health=50;y.fireProcs(Trigger::OnDash,nullptr);check(y.player.health==57,"A proc heals");
        Combat z;z.perks.procs.push_back(masteryProc(Trigger::OnDash,ProcAction::Heal,7,0.f));
        z.player.health=50;for(int i=0;i<50;++i)z.fireProcs(Trigger::OnDash,nullptr);check(z.player.health==50,"Chance 0 never fires");
        check(y.perks.procs.size()==1,"The proc list belongs to the combat state");
    });
    test("Special-effect nodes bought in the tree reach combat through the real triggers",[]{
        Combat g;g.player.equippedJobs={{2,2}};g.restart();                 // Flash Executioner
        g.mastery.xp[1]=5000;
        for(int n:{0,1,2,3,4,5,6,7,8,9,10,12})check(g.mastery.buy(2,n),"Nodes bought in tree order (LIGHT BURST, RADIANT END)");
        g.refreshStats();
        check(g.perks.procs.size()==3,"Light Burst (1) and Radiant End (2) are active");
        g.stats.critChance=1.f;
        Enemy& e=g.enemies[0];e.health=e.maxHealth=100000;e.wake=0;
        g.hit(e,10,false);
        check(g.player.storedLightLances>=1,"A critical hit stored a lance (LIGHT BURST)");
        const int lancesBefore=g.player.storedLightLances;
        Enemy& k=g.enemies[1];k.health=k.maxHealth=5;k.wake=0;
        g.player.health=40;g.hit(k,100000,false);
        check(g.player.health==42,"A kill heals 2 (RADIANT END)");
        check(g.player.storedLightLances>lancesBefore,"A kill also stores a lance");
        g.player.invulnerable=0;g.damagePlayer(100000);g.restart();
        check(g.perks.procs.empty()&&g.mastery.xpOf(2)==0,"A lost run removes the special effects too");
    });
    test("Altar menu controller: tabs, tap-to-select, second tap or BUY buys, dragging scrolls without buying",[]{
        namespace M=MemoryMenu;
        Combat g;g.memoryMenu=true;
        MemoryMenuController c;
        c.down(g,M::panelX+300,M::panelY+80);
        check(g.memoryTab==1,"The run tab button switches tabs");
        const auto& tree=masteryTree(g.activeJob());
        float nx=0,ny=0;M::nodeRect(tree[0],g.memoryTreeScrollX,g.memoryTreeScrollY,nx,ny);
        c.down(g,nx+10,ny+10);c.up(g,nx+10,ny+10);
        check(g.memoryTreeSelected==0,"A tap selects the node");
        g.mastery.xp[Progression::jobIndex(g.activeJob())]=24;
        c.down(g,nx+10,ny+10);c.up(g,nx+10,ny+10);
        check(g.mastery.has(g.activeJob(),0),"A second tap on the selected node buys it");
        float n1x=0,n1y=0;M::nodeRect(tree[1],g.memoryTreeScrollX,g.memoryTreeScrollY,n1x,n1y);
        c.down(g,n1x+10,n1y+10);c.up(g,n1x+10,n1y+10);
        check(g.memoryTreeSelected==1&&!g.mastery.has(g.activeJob(),1),"Another tap only selects");
        c.down(g,M::buyX+10,M::buyY+10);
        check(g.mastery.has(g.activeJob(),1),"The BUY button buys the selected node");
        // dragging
        const float before=g.memoryTreeScrollX;
        c.down(g,M::treeX+30,M::treeY+M::treeH-20);
        c.move(g,M::treeX+30-60,M::treeY+M::treeH-20);
        c.up(g,M::treeX+30-60,M::treeY+M::treeH-20);
        check(g.memoryTreeScrollX>before+1.f,"Dragging left scrolls the tree to the right");
        check(g.memoryTreeSelected==1,"A drag selects and buys nothing");
        for(int i=0;i<40;++i){c.down(g,M::treeX+300,M::treeY+100);c.move(g,M::treeX-500,M::treeY+100);c.up(g,M::treeX-500,M::treeY+100);}
        float left=0,right=0,bottom=0;M::treeExtent(g.activeJob(),left,right,bottom);
        check(g.memoryTreeScrollX<=right-M::treeW*.5f+.01f&&g.memoryTreeScrollX>=left+M::treeW*.5f-.01f,"Scrolling is clamped to the tree");
        // permanent tab rows and close
        c.down(g,M::panelX+100,M::panelY+80);
        check(g.memoryTab==0,"The permanent tab button switches back");
        g.progression.memories=100;
        c.down(g,M::panelX+100,M::rowY+8);
        check(g.progression.level(g.activeJob(),Stat::Attack)==1,"A tap on a permanent row raises that stat");
        c.down(g,5,5);
        check(!g.memoryMenu,"A tap outside the panel closes the menu");
    });
    std::cout<<total-failed<<'/'<<total<<" passed\n";return failed?1:0;
}
