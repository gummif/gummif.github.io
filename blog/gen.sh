# bin/sh

name="Gudmundur's Blog"

pandoc -s -o index.html --metadata pagetitle="$name" -c style.css _index.md
pandoc -s -o contract_code_gen.html --metadata pagetitle="$name -- Contract code gen" -c style.css contract_code_gen.md

