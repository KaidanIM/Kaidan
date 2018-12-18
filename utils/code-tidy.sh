#!/usr/bin/env bash
# Tidy up code to use modern C++

KAIDAN_SOURCES=$(dirname "$(readlink -f "${0}")")/..

mkdir -p .tidy-tmp
cd .tidy-tmp

cmake $KAIDAN_SOURCES -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# will output an error about moc files: just ignore, we don't want to tidy moc generated files
run-clang-tidy -header-filter='.*' -checks='-*,modernize-avoid-bind,modernize-deprecated-headers,modernize-loop-convert,modernize-make-shared,modernize-make-unique,modernize-pass-by-value,modernize-raw-string-literal,modernize-redundant-void-arg,modernize-replace-auto-ptr,modernize-shrink-to-fit,modernize-use-auto,modernize-use-bool-literals,modernize-use-default,modernize-use-emplace,modernize-use-nullptr,modernize-use-using' -fix

git -C $KAIDAN_SOURCES checkout -- src/singleapp

cd ..
rm -rf .tidy-tmp

