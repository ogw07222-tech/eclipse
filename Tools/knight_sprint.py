"""Draw the knight's sprint on a fixed pixel grid from continuous leg paths."""
import math

from PIL import Image, ImageDraw

SPRINT_FRAMES = 54
SPRINT_START_ROW = 27
SPRINT_ROWS = 4


def sprint_poses(atlas):
    # Preserve the original helmet, arm, sword, and cape as a registered layer.
    torso = atlas.crop((5 * 64, 2 * 94, 6 * 64, 3 * 94))
    clear = atlas.info['transparency']
    torso.paste(clear, (0, 70, 64, 94))
    source = atlas.crop((5 * 64, 2 * 94, 6 * 64, 3 * 94))
    blade = Image.new('L', (64, 94))
    ImageDraw.Draw(blade).polygon([(30, 63), (54, 71), (55, 75), (31, 68)], fill=255)
    torso.paste(source, (0, 0), blade)

    palette = atlas.getpalette()
    def ink(rgb):
        return min((i for i in range(len(palette)//3) if i != clear),
                   key=lambda i: sum((palette[3*i+j]-value)**2 for j,value in enumerate(rgb)))

    front = tuple(map(ink, [(23,24,31), (53,54,64), (94,94,99), (138,135,128)]))
    rear = tuple(map(ink, [(18,19,25), (38,39,49), (59,61,73), (88,87,96)]))

    def leg(draw, phase, colors):
        # On contact, the foot travels backward at the exact ground speed.
        travel = 120 / 1.85
        stance = .48
        stride = travel * stance
        front_x = 48.
        if phase < stance:
            x, y, angle = front_x - phase*travel, 84., 0.
        else:
            t = (phase-stance)/(1-stance)
            # Hermite swing retains the ground-speed tangent at both contacts.
            x = front_x-stride + stride*(3*t*t-2*t*t*t) - travel*(1-stance)*t*(1-t)*(1-2*t)
            y = 84.-13.*math.sin(math.pi*t)**2
            angle = -.58*math.sin(2*math.pi*t)

        hip = (33., 66.)
        dx, dy = x-hip[0], y-hip[1]
        length = math.hypot(dx,dy)
        rise = math.sqrt(max(0., 13.25**2-(length*.5)**2))
        knee = (hip[0]+dx*.5+dy/length*rise, hip[1]+dy*.5-dx/length*rise)
        ankle = (x,y)
        dark, shade, steel, shine = colors
        def plate(a,b,wa,wb):
            ux,uy=b[0]-a[0],b[1]-a[1]
            size=math.hypot(ux,uy)
            nx,ny=uy/size,-ux/size
            outline=[(a[0]+nx*wa,a[1]+ny*wa),(b[0]+nx*wb,b[1]+ny*wb),
                     (b[0]-nx*wb,b[1]-ny*wb),(a[0]-nx*wa,a[1]-ny*wa)]
            draw.polygon(outline,fill=shade,outline=dark)
            for start, end in ((.13,.40),(.49,.81)):
                p = (a[0]+ux*start+nx,a[1]+uy*start+ny)
                q = (a[0]+ux*end+nx,a[1]+uy*end+ny)
                draw.line((p,q),fill=steel,width=2)
                draw.point(p,fill=shine)
        plate(hip,knee,3.1,2.65)
        plate(knee,ankle,2.55,1.7)
        kx,ky=knee
        draw.line((kx-1,ky,kx+1,ky-1),fill=shine)
        ca,sa=math.cos(angle),math.sin(angle)
        boot=[(x+u*ca-v*sa,y+u*sa+v*ca) for u,v in [(-2,-1),(1,-1),(2,1),(5,1),(6,3),(-2,3)]]
        draw.polygon(boot,fill=shade,outline=dark)
        draw.line((boot[2],boot[3]),fill=steel)

    for i in range(SPRINT_FRAMES):
        pose = Image.new('P',(64,94),clear)
        pose.putpalette(palette)
        pose.info['transparency'] = clear
        draw = ImageDraw.Draw(pose)
        phase = i / SPRINT_FRAMES
        leg(draw,(phase+.5)%1,rear)
        leg(draw,phase,front)
        pose.paste(torso,(0,0),torso.convert('RGBA').getchannel('A'))
        yield pose
