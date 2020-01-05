# bin/sh

name="Gudmundur's Blog"

pandoc -s -o index.html --metadata pagetitle="$name" -c style.css _index.md

pandoc -s -o contract_code_gen.html --metadata pagetitle="$name -- Code Generation with C++ Contracts" -c style.css contract_code_gen.md

# gcc -std=c++17 -lstdc++ lo1.cpp && ./a.out
pandoc -s -o overloading_lambdas.html --metadata pagetitle="$name -- Overloading Lambdas and Sharing State" -c style.css overloading_lambdas.md

#pandoc -s -o abusing_enum.html --metadata pagetitle="$name -- (Ab)using enum class for strong type aliases" -c style.css abusing_enum.md
