"""Draw the knight's sprint on a fixed pixel grid from continuous leg paths."""
import math

from PIL import Image, ImageDraw

SPRINT_FRAMES = 24
SPRINT_START_ROW = 27
SPRINT_ROWS = 4

SPRINT_WORLD_STRIDE = 196.
SPRINT_DURATION = .7
SPRINT_STANCE = .26
SPRINT_SEGMENT_LENGTH = 14.


def sprint_leg(phase):
    """Continuous source-space joints; bright right and dark left differ by .5."""
    phase %= 1.
    travel = SPRINT_WORLD_STRIDE / 1.85
    stance_travel = travel * SPRINT_STANCE
    front_x = 47.
    if phase < SPRINT_STANCE:
        x, y, angle = front_x - phase * travel, 84., 0.
    else:
        t = (phase - SPRINT_STANCE) / (1. - SPRINT_STANCE)
        x = (front_x - stance_travel + stance_travel * (3*t*t - 2*t*t*t)
             - travel * (1. - SPRINT_STANCE) * t * (1-t) * (1-2*t))
        # Rear recovery stays below the pelvis/sword instead of folding up into
        # their opaque pixels. Lower the foot as it recovers in front.
        front_weight = max(0., min(1., (x - 30.) / 18.))
        front_weight = front_weight * front_weight * (3 - 2*front_weight)
        y = 84. - 7. * math.sin(math.pi*t)**2 * (1 - .75*front_weight)
        angle = -.18 * math.sin(2*math.pi*t)
    hip = (33., 66.)
    dx, dy = x-hip[0], y-hip[1]
    reach = math.hypot(dx, dy)
    if not 0 < reach <= 2 * SPRINT_SEGMENT_LENGTH:
        raise ValueError(('Unreachable sprint ankle', phase, x, y))
    rise = math.sqrt(SPRINT_SEGMENT_LENGTH**2 - (reach/2)**2)
    knee = (hip[0] + dx/2 + dy/reach*rise,
            hip[1] + dy/2 - dx/reach*rise)
    return dict(hip=hip, knee=knee, ankle=(x,y), boot_angle=angle,
                contact=phase < SPRINT_STANCE)


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
        joints = sprint_leg(phase)
        hip, knee, ankle = joints['hip'], joints['knee'], joints['ankle']
        x, y = ankle
        angle = joints['boot_angle']
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


def bake_sprint_only():
    """Replace only the reserved sprint rows; never rebake attacks/other clips."""
    import hashlib
    import json
    from pathlib import Path
    root = Path(__file__).resolve().parents[1]
    asset = root/'Assets/Resources/Eclipse/knight_motion.png'
    atlas = Image.open(asset)
    if atlas.mode != 'P' or atlas.size != (1024, 2914):
        raise ValueError('Expected original indexed1024x2914 knight atlas')
    before = atlas.convert('RGBA')
    frames = list(sprint_poses(atlas))
    if len({hashlib.sha256(f.convert('RGBA').tobytes()).hexdigest() for f in frames}) != 24:
        raise ValueError('Sprint poses must be24 distinct PNG images')
    updated = atlas.copy()
    updated.paste(atlas.info['transparency'], (0, SPRINT_START_ROW*94, 1024, 31*94))
    for i, frame in enumerate(frames):
        updated.paste(frame, (i%16*64, (SPRINT_START_ROW+i//16)*94))
    if updated.convert('RGBA').crop((0,0,1024,SPRINT_START_ROW*94)).tobytes() != before.crop((0,0,1024,SPRINT_START_ROW*94)).tobytes():
        raise ValueError('Non-sprint pixels changed')
    updated.save(asset, optimize=True)
    art = root/'Art/knight/sprint'
    art.mkdir(parents=True, exist_ok=True)
    strip = Image.new('RGBA', (512, 282))
    poses = []
    for i, frame in enumerate(frames):
        strip.alpha_composite(frame.convert('RGBA'), (i%8*64, i//8*94))
        poses.append(dict(frame=432+i, right=sprint_leg(i/24), left=sprint_leg(i/24+.5)))
    strip.save(art/'authored_run_24.png', optimize=True)
    (art/'run.json').write_text(json.dumps(dict(start=432,count=24,duration=.7,
        world_stride=196,base_pose_fps=24/.7,cell=[64,94],pivot=[28,88],
        segment_length=14,heel_lift_max=7,right='bright foreground',left='dark rear',
        leg_horizontal_scale=1.,poses=poses,asset_sha256=hashlib.sha256(asset.read_bytes()).hexdigest(),
        validation_scope='PNG/joint geometry inspection only; no full game or owner test suite'),indent=2)+'\n')
    print('Baked24 distinct sprint PNG poses; all non-sprint pixels preserved.')


if __name__ == '__main__':
    import argparse
    parser = argparse.ArgumentParser(description='Bake only the standard knight sprint PNG region')
    parser.add_argument('--bake-only', action='store_true', required=True)
    parser.parse_args()
    bake_sprint_only()
