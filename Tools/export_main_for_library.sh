#!/usr/bin/env bash
# Export exact origin/main tracked files as a verified snapshot for ChatGPT Library.
set -Eeuo pipefail
cd "${1:-$HOME/eclipse}"
[[ -d .git ]] || { echo "ERROR: no ECLIPSE git checkout" >&2; exit 1; }
[[ $(git remote get-url origin) == *ogw07222-tech/eclipse* ]] || { echo "ERROR: wrong remote" >&2; exit 1; }
git fetch origin main
commit=$(git rev-parse 'origin/main^{commit}')
out="${2:-$HOME/eclipse-main-${commit:0:12}.zip}"
python - "$commit" "$out" <<'PY'
import hashlib, io, json, pathlib, subprocess, sys, tarfile, zipfile
commit, output = sys.argv[1:]
tar_bytes = subprocess.run(
    ['git', 'archive', '--format=tar', commit],
    check=True, capture_output=True).stdout
files = {}
with tarfile.open(fileobj=io.BytesIO(tar_bytes)) as archive:
    for member in archive.getmembers():
        if member.isfile():
            files[member.name] = archive.extractfile(member).read()
manifest = {
    'repository': 'ogw07222-tech/eclipse',
    'branch': 'main', 'commit': commit,
    'file_count': len(files),
    'total_uncompressed_bytes': sum(len(data) for data in files.values()),
    'files': {name: {'bytes': len(data),
                     'sha256': hashlib.sha256(data).hexdigest()}
              for name, data in sorted(files.items())}
}
path = pathlib.Path(output).expanduser()
path.parent.mkdir(parents=True, exist_ok=True)
with zipfile.ZipFile(path, 'w', compression=zipfile.ZIP_DEFLATED, compresslevel=6) as zip_out:
    for name, data in sorted(files.items()):
        zip_out.writestr('eclipse-main/' + name, data)
    zip_out.writestr('eclipse-main-snapshot.json',
                     json.dumps(manifest, ensure_ascii=False, indent=2) + '\n')
with zipfile.ZipFile(path) as verify:
    invalid = verify.testzip()
    if invalid:
        raise ValueError('ZIP checksum failure: ' + invalid)
print('FILE:', path.resolve())
print('COMMIT:', commit)
print('TRACKED FILES:', len(files))
print('ZIP SHA256:', hashlib.sha256(path.read_bytes()).hexdigest())
print('Upload this ZIP to ChatGPT Library /ECLIPSE to create the actual snapshot.')
PY
