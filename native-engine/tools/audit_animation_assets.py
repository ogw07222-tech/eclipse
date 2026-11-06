#!/usr/bin/env python3
"""Read-only PNG inspection; writes reports only, never runs game/tests/builds.

Preview sequences are isolated clip samples at default speed, without hit-stop,
state interruption, terrain or velocity curves. They are not runtime evidence.
"""
from pathlib import Path
import hashlib, json, math, re
from PIL import Image
ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / 'native-engine/animation-audit'
CLIPS = []

def add(actor, name, path, cell, columns, pivot, before, duration, after=None,
        new_duration=None, note='', variable=False, code=False, before_sheet=None, before_columns=None, new_png=False):
    atlas = Image.open(ROOT/'Assets/Resources/Eclipse'/path).convert('RGBA')
    w,h = cell
    sequences = []
    for side,sequence in enumerate((before, before if after is None else after)):
        current=Image.open(ROOT/before_sheet).convert('RGBA') if side==0 and before_sheet else atlas
        sequence_columns=before_columns if side==0 and before_columns else columns
        hashes, boxes = [], []
        for index in sequence:
            x,y = index%sequence_columns*w,index//sequence_columns*h
            if x<0 or y<0 or x+w>current.width or y+h>current.height:
                raise ValueError((path,index,'outside atlas'))
            frame = current.crop((x,y,x+w,y+h))
            box = frame.getbbox()
            if not box and not (path=='ui/white_night_motion.png' and index==59):
                raise ValueError((path,index,'unexpected empty frame'))
            hashes.append(hashlib.sha256(frame.tobytes()).hexdigest())
            boxes.append(box)
        sequences.append(dict(frames=sequence, slots=len(sequence), unique=len(set(hashes)),
                              adjacent_identical=sum(a==b for a,b in zip(hashes,hashes[1:])),
                              bounds=boxes, loop_end_identical=hashes[0]==hashes[-1]))
    if before_sheet:sequences[0].update(repo_path=before_sheet,columns=before_columns or columns)
    after_duration = duration if new_duration is None else new_duration
    CLIPS.append(dict(actor=actor,name=name,path=path,cell=cell,columns=columns,pivot=pivot,
                      before=sequences[0],after=sequences[1],duration=duration,
                      after_duration=after_duration,variable=variable,new_png=new_png,code=code,note=note,
                      verification='PNG bounds/empty-frame inventory/hash inspection only; native playback, foot contact, flicker, transitions and device rendering unverified'))

def doubled(start,count,offset,loop=False):
    return [j for i in range(count) for j in ([start+i,start+i+offset] if loop or i+1<count else [start+i,start+i])]

def main():
    knight='knight_motion.png';cell=(64,94);pivot=(28,88)
    definitions=[('대기',0,8,.96),('점프',48,6,.52),('낙하',64,6,.65),
                 ('강베기',128,10,.34),('찌르기',144,10,.42),('착지',80,6,.12),
                 ('대시',96,8,.15),('사망',176,12,.9),('내려찍기',192,8,.38)]
    for name,start,count,d in definitions:
        air=name in ('점프','낙하')
        seq=list(range(start,start+count)) if air else doubled(start,count,224,name=='대기')
        note='속도/상태 기반: 명목 시간으로 일정 FPS를 보장할 수 없음' if air or name=='내려찍기' else '기존 키 자세+중간 자세 유지. FPS는 키 자세 수가 아닌 표시 슬롯 기준.'
        add('일반 기사',name,knight,cell,16,pivot,seq,d,note=note,variable=air or name=='내려찍기')
    add('일반 기사','달리기',knight,cell,16,pivot,list(range(24)),.7,after=list(range(432,456)),before_sheet='Art/knight/sprint/before_displayed_24.png',before_columns=8,new_png=True,code=True,note='기존54장 중24선택→새24PNG432–455. 승인24장/0.7초 유지. 관절분절14px, 발 궤적120→실제196단위에 동기화. 이동 거리 기반.',variable=True)
    add('일반 기사','약베기',knight,cell,16,pivot,list(range(112,128)),.23,
        after=[112+int(i) for i in re.search(r'knightWeakPlayback\[\]=\{([^}]+)\}',(ROOT/'native-engine/src/motion.hpp').read_text()).group(1).split(',')],code=True,
        note='16→11 기존 자세 선택. 접촉 위상 .43에서 원래 6번 자세. 타격/게임플레이 타이밍 유지. 검 궤적은 표시 PNG 자세와 동기화. 기본 속도 기준; 장비/강화/히트스톱은 실제 속도에 영향.')
    for i,(name,d) in enumerate([('스킬1 찌르기',.48),('스킬2 휩쓸기',.415),('스킬3 준비',.40)]):
        add('일반 기사',name,'knight_skill_motion.png',(112,94),4,(44,88),doubled(i*4,4,12),d,
            after=list(range(i*12,i*12+12)),before_sheet='Art/knight/skill/original_4x6.png',
            before_columns=4,new_png=True,code=True,
            note='기존 GitHub 키/중간 자세에서 파생한12고유 PNG 자세. 상체·검·망토 픽셀에 제한적 정수 워프를 적용, 발 접지 영역 보존. 검기는 별도 PNG 유지. 실제 자연스러움 미검증.')
    mage='mage_motion.png'
    for name,row,d in [('대기',0,.96),('공격 지상',8,.32),('공격 공중',9,.32),('점프',2,.52),('낙하',3,.65),('착지',4,.12),('대시',5,.15),('사망',7,.9)]:
        air=name in ('점프','낙하')
        indices=[0,1,2,4,5,7] if air or name=='착지' else list(range(8))
        add('마법사',name,mage,cell,8,pivot,[row*8+i for i in indices],d,
            after=list(range(row*8,row*8+8)) if air else None,variable=air,code=air,
            note='기존 8자세를 연속 수직 속도 위상에서 직접 선택. 새 PNG/프레임 증식 아님. 자세가 나올 시간은 실제 물리 궤적에 의존.' if air else '기존 유지. 대기/공격 8자세. 기본 공격은 .32초 타이머에서 직접 매핑.')
    add('마법사','달리기','mage_run.png',cell,16,pivot,list(range(16)),.7,variable=True,note='기존 16장/.7초 유지, 196단위 거리 기반.')
    white='ui/white_night_motion.png'
    add('백야의 집행자','달리기',white,(64,96),8,(32,88),list(range(88,120)),.7,new_duration=1.,code=True,variable=True,
        note='기존 32장 유지, 표시 주기만 196→280단위. 기본속도 280에서 45.71→32FPS. 발 미끄러짐과 장비/아날로그 속도별 시각 검증 미완료.')
    for name,start,count,d in [('기본 베기',16,15,.46),('돌진',31,12,.5),('강화',43,12,.46),('점멸',55,12,.36),('판결',67,20,1.12)]:
        add('백야의 집행자',name,white,(64,96),8,(32,88),list(range(start,start+count)),d,note='기존 유지. 돌진 24FPS/판결17.86FPS: 실제 연결 검증과 필요 구간 재작화 미완료. 기본 베기는 장비/강화에 따라 단축.' if name in ('돌진','판결','기본 베기') else '기존 유지; 실제 상태 전환 검증 미완료. 점멸59번은 완전 투명(.12~.15초), 접촉 전 소멸 연출로 추정되지만 의도 확인 필요.')
    add('백야의 집행자','점프·낙하',white,(64,96),8,(32,88),list(range(8,15)),1.,variable=True,note='vy -650~650을 7자세로 매핑. 1초는 뷰어용 기준이며 런타임 고정 지속시간/FPS 아님.')
    for label,file,speed,d in [('초록','green',95,.72),('빨강','red',63,1.10),('보라','purple',49,1.16)]:
        for move in (True,False):
            duration=64/speed if move else d
            count=math.ceil(duration*(16 if move else 24))
            add(label+' 슬라임','이동' if move else '공격',f'monsters/slime_{file}.png',(128,128),4,(64,116),list(range(0,8)) if move else list(range(8,16)),duration,variable=move,
                note=f'8장 유지 / 미완료. 최저 목표에 최소 {count}고유 자세 필요. PNG 실루엣이 slime_shapes.hpp 충돌 판정에 연결됨: 별도 시각 매핑 설계 없이 재작화/판정 변경 금지.')
    OUT.mkdir(parents=True,exist_ok=True)
    (OUT/'animation-audit.json').write_text('{\n  "base":"knight-skill-12",\n  "status":"INCOMPLETE — static inspection and partial presentation fixes only",\n  "clips":[\n'+',\n'.join(json.dumps(c,ensure_ascii=False) for c in CLIPS)+'\n]}\n')
    lines=['# 캐릭터 애니메이션 점검 — 미완료','',
           '최신 기준: GitHub 본편 PNG 정적 점검. 신규 기사 스킬 12자세와 별도 검기 시트 포함. PNG 범위·빈 프레임·RGBA 해시를 정적으로 확인했다. 빌드/게임 실행/테스트/GitHub Actions는 실행하지 않았다. 자연스러움·발 접촉·반짝임·전환·기기 재생은 검증하지 않았다. 일반 기사 달리기24PNG 및 스킬 3종 각12PNG를 반영했다. 그 밖의 신규 PNG 미완료 항목은 그대로다. 전체 최적화 완료가 아니다. 후속 변경 검토는 [CHANGED_MOTION_REVIEW.md](CHANGED_MOTION_REVIEW.md)를 참고한다.','',
           'FPS는 기본 이동속도 280·기본 장비·히트스톱 없는 조건의 표시 슬롯/초다. 기존 보고의 키 프레임 수에는 중간 PNG 자세가 빠져 있었다. 중간 자세를 사용하는 모션은 슬롯 수와 고유 이미지 수를 함께 적었다. 마지막 자세 홀드 슬롯은 중복이며 새 제작 프레임으로 계산하지 않는다. 60Hz 표시에서는 60FPS 초과 슬롯 일부가 보이지 않을 수 있다. 점프/낙하는 속도 기반이라 표의 FPS를 고정 재생속도로 해석하면 안 된다.','',
           '| 캐릭터 | 동작 | 기존 프레임(슬롯/고유) | 기존 FPS | 변경 프레임(슬롯/고유) | 변경 FPS | 새 PNG | 코드 반영 | 미검증/남은 작업 |',
           '|---|---|---:|---:|---:|---:|---|---|---|']
    for c in CLIPS:
        b,a=c['before'],c['after'];var=c['name'] in ('점프','낙하','점프·낙하','내려찍기')
        bf='가변' if var else f"{b['slots']/c['duration']:.2f}"
        af='가변' if var else f"{a['slots']/c['after_duration']:.2f}"
        lines.append(f"|{c['actor']}|{c['name']}|{b['slots']}/{b['unique']}|{bf}|{a['slots']}/{a['unique']}|{af}|{'예' if c['new_png'] else '아니오'}|{'변경' if c['code'] else '유지'}|{c['note']} 실제 재생 미검증.|")
    lines += ['', '## 정적 확인 범위', '',
              '- 위 표의 모든 참조 프레임이 실제 PNG 안에 있는지 확인했다. 백야 점멸59번은 완전 투명하며 나머지 참조 프레임은 비어 있지 않다. 59번은 접촉 직전(.12~.15초) 소멸 연출로 추정되지만 실제 의도와 연결은 미검증이다. `animation-audit.json`에 각 자세의 경계와 고유 수를 기록했다.',
              '- 일반 기사 달리기 PNG만 수정했다. 충돌 판정 데이터는 변경하지 않았다. 이동속도·대미지·쿨타임·공격 판정 타이머는 변경하지 않았다.',
              '- `posePhase`는 기존 계산 결과를 표시용으로 노출하며, 마법사 공중 자세 선택과 일반 기사 약베기 선택에만 사용한다.',
              '- 생성한 48자세 스킬 후보는 2회 모두 원본 검 길이·픽셀/디자인 일관성을 충족하지 못해 채택하지 않았다. 새 PNG 제작과 통합은 미완료다.',
              '- 일반 기사 강베기/찌르기는 중간 자세까지 실제로 참조한다. 강베기의 키 기준 29.41FPS와 표시 슬롯 기준58.82FPS를 혼동하면 안 된다. 정상 동작 보존 요구에 따라 이번에는 유지했다.',
              '- 슬라임의 PNG 수정은 판정 실루엣을 재생성하면 게임플레이에 영향을 준다. 현재 판정을 고정하는 별도 시각용 아틀라스/매핑이 먼저 필요하다.',
              '- 백야의 집행자 달리기 주기 변경은 정적 FPS 계산만 완료했다. 지면 미끄러짐이 악화되는지 확인되지 않아 전체 작업은 초안 상태다.',
              '', '## 검토용 뷰어', '',
              '`native-engine/preview/animations.html`은 원본 PNG를 정적 보고의 순서로 재생하는 보조 도구다. 실제 게임 실행 결과가 아니다. 점프/낙하는 선형 위상으로 보여주며 실제 중력 궤적을 재현하지 않는다. 공격 중단·히트스톱·물리·이동·충돌·이펙트는 재현하지 않는다.',
              '', '## 재생성', '', '`python3 native-engine/tools/audit_animation_assets.py` (Pillow 필요). 에셋을 읽어 보고서만 쓴다. 게임이나 테스트는 실행하지 않는다.']
    (OUT/'ANIMATION_AUDIT.md').write_text('\n'.join(lines)+'\n')
    print(f'Inspected {len(CLIPS)} clips; PNG indices in bounds; intentional-candidate blink frame 59 is empty. Runtime/visual acceptance NOT verified.')

if __name__=='__main__':main()
