# Runs the valgrind profiling suite (scripts/profile.sh) inside the Ubuntu
# container from docker/Dockerfile, on Windows with Docker Desktop.
#
#   powershell -ExecutionPolicy Bypass -File scripts\docker_profile.ps1
#   powershell -File scripts\docker_profile.ps1 chapters cases
#   powershell -File scripts\docker_profile.ps1 -Develop D:\src callgrind
#
# The checkout is expected at <Develop>\investm (default C:\develop\investm).
# Reports land in <Develop>\investm\build\profile\reports.
param(
  [string]$Develop = "C:\develop",
  [Parameter(ValueFromRemainingArguments = $true)][string[]]$Tools
)
$ErrorActionPreference = "Stop"
$repo = Join-Path $Develop "investm"
docker build -t investm-dev -f (Join-Path $repo "docker\Dockerfile") $repo
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
docker run --rm -v "${Develop}:/develop" -w /develop/investm investm-dev `
  scripts/profile.sh @Tools
exit $LASTEXITCODE
