Write-Host "[1/2] Configuring CMake..." -ForegroundColor Cyan
cmake -G "MinGW Makefiles" -B build
if ($LASTEXITCODE -ne 0) {
    Write-Host "[ERROR] CMake configuration failed!" -ForegroundColor Red
    exit $LASTEXITCODE
}

Write-Host "[2/2] Building project..." -ForegroundColor Cyan
cmake --build build -j $env:NUMBER_OF_PROCESSORS
if ($LASTEXITCODE -ne 0) {
    Write-Host "[ERROR] Build failed!" -ForegroundColor Red
    exit $LASTEXITCODE
}

Write-Host "[SUCCESS] Build completed successfully!" -ForegroundColor Green
