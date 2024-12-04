#!/bin/sh

cp githooks/pre-commit .git/hooks/pre-commit

chmod +x .git/hooks/pre-commit

echo "Pre-commit hook installed."
