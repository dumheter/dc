$files = Get-ChildItem -Path . -File -Recurse -Include "*.hpp","*.cpp","*.h" |
where {
      $_.FullName.Contains("dc\include") -or
      $_.FullName.Contains("dc\src") -or
      $_.FullName.Contains("dc\tests")
}

foreach ($file in $files) { clang-format -style=file -i $file.FullName } 
