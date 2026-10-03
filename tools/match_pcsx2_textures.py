"""Match exported originals to PCSX2 dumps without guessing hash filenames.
Usage: python match_pcsx2_textures.py ASSETS DUMPS REPLACEMENTS
Requires Pillow + numpy. Writes ASSETS/replacements.tsv and match-report.json.
Only exact RGB matches at source 5-bit precision are accepted automatically.
If several different hash names match, record ambiguity instead of choosing.
"""
from pathlib import Path
import argparse,hashlib,json,os,time,tempfile
import numpy as np
from PIL import Image

def fingerprint(path):
    with Image.open(path) as im:
        rgb=np.array(im.convert('RGB'),dtype=np.uint8)
    return (rgb.shape[1],rgb.shape[0],hashlib.sha256((rgb>>3).tobytes()).hexdigest())

def atomic_text(path,text):
    if path.exists() and path.read_text()==text:return
    fd,name=tempfile.mkstemp(prefix=path.name+'.',suffix='.tmp',dir=path.parent)
    try:
        with os.fdopen(fd,'w') as stream:stream.write(text)
        os.replace(name,path)
    finally:
        if os.path.exists(name):os.unlink(name)

def scan(a,cache):
    keys={};errors=[]
    for path in a.dumps.rglob('*'):
        if path.suffix.lower() not in ('.png','.jpg','.jpeg','.bmp','.webp'):continue
        try:
            stat=path.stat();signature=(stat.st_size,stat.st_mtime_ns)
            if path not in cache or cache[path][0]!=signature:cache[path]=(signature,fingerprint(path))
            keys.setdefault(cache[path][1],[]).append(path.relative_to(a.dumps).with_suffix('.png').as_posix())
        except Exception as e:errors.append(dict(file=str(path),error=str(e)))
    existing={}
    mapping=a.assets/'replacements.tsv'
    if mapping.exists():
        for line in mapping.read_text().splitlines():
            parts=line.split('\t')
            if len(parts)==2:existing[parts[0]]=parts[1]
    rows=[];report=[]
    for path in sorted((a.assets/'textures').glob('*.png')):
        name=path.name[:-4];matches=sorted(set(keys.get(fingerprint(path),[])))
        status='matched' if len(matches)==1 else 'ambiguous' if matches else 'no exact dump match'
        chosen=existing.get(name) or (matches[0] if len(matches)==1 else None)
        if chosen:rows.append(name+'\t'+chosen)
        report.append(dict(material=name,status=status,candidates=matches,mapping=chosen,replacementExists=bool(chosen and (a.replacements/chosen).is_file())))
    # Preserve manually assigned materials absent from the supplied texture archive.
    known={r['material'] for r in report}
    rows.extend(n+'\t'+v for n,v in existing.items() if n not in known)
    atomic_text(mapping,'\n'.join(rows)+'\n')
    atomic_text(a.assets/'match-report.json',json.dumps(dict(materials=report,unreadable=errors),indent=2))
    return len(rows),len(report),len(errors)

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('assets',type=Path);p.add_argument('dumps',type=Path);p.add_argument('replacements',type=Path);p.add_argument('--watch',action='store_true');a=p.parse_args()
    cache={};previous=None
    while True:
        try:
            result=scan(a,cache)
            if result!=previous:print('Mapped %d of %d textures. Unreadable dumps: %d'%result,flush=True);previous=result
        except (OSError,ValueError) as e:
            if not a.watch:raise
            print('Will retry:',e,flush=True)
        if not a.watch:break
        time.sleep(5)
if __name__=='__main__':
    try:main()
    except KeyboardInterrupt:pass
