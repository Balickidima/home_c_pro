Write-Host "Compiling Snake Game..."
$gccPath = "C:\msys64\mingw64\bin\gcc.exe"
$sourceFile = "main.c"
$outputFile = "snake.exe"
$library = "-lncursesw"

if (Test-Path $gccPath) {
    Write-Host "Using GCC at: $gccPath"
    & $gccPath $sourceFile -o $outputFile $library
    
    if ($LASTEXITCODE -eq 0) {
        Write-Host "Compilation successful!"
        Write-Host "You can run the game with: .\$outputFile"
    } else {
        Write-Host "Compilation failed with error code: $LASTEXITCODE"
    }
} else {
    Write-Host "Error: GCC not found at $gccPath"
    Write-Host "Please install MSYS2 and mingw-w64-x86_64-gcc"
}