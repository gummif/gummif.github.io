#!/bin/bash

# usage: ./html_results.sh

branch="master"
if [ $# -eq 1 ]; then 
    branch="$1"
fi
"%Y-%m-%d"
baseurl="http://gummif.github.io/FAO"
file="./$branch/index.html"
touch "$file"
#gitmsg=$(git log -1 --date=short --pretty="%ad")  # %h %s
gitmsg=$(date "+%Y-%m-%d")

cat >"$file" <<EOF
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
	"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">

<head>
	<title>FastArrayOps.jl</title>
	<meta http-equiv="content-type" content="text/html;charset=utf-8" />
	<meta name="generator" content="Geany 1.24.1" />
</head>
<body>

EOF

cat >>"$file" <<EOF
<h4>Benchmarks $gitmsg ($branch) for FastArrayOps.jl</h4>

EOF
for f in "./$branch/"*.svg
do
	echo "<img src=\"$baseurl${f:1}\"></img>" >>"$file"
done

cat >>"$file" <<EOF

</body>
</html>

EOF

exit
