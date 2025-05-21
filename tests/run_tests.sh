#!/bin/sh
set -e
cd "$(dirname "$0")"

# ensure Plan 9 'mk' build tool is available
if ! command -v mk >/dev/null 2>&1; then
  echo "Error: 'mk' not found in PATH." >&2
  echo "Install plan9port and ensure mk is in your PATH (try scripts/install_deps.sh)." >&2
  exit 1
fi
mk clean >/dev/null
mk parser_test parseurl_test fetch_url_test >/tmp/test_build.log && tail -n 20 /tmp/test_build.log

# run parser_test
./parser_test > /tmp/parser_test.out
if grep -q '^ok' /tmp/parser_test.out; then
  echo "parser_test passed"
else
  echo "parser_test failed" >&2
  cat /tmp/parser_test.out >&2
  exit 1
fi

# start simple http server for fetch_url_test
PORT=8888
python3 -m http.server $PORT --directory .. >/tmp/http.log 2>&1 &
server_pid=$!
sleep 1

./fetch_url_test "http://127.0.0.1:$PORT/README.md" > /tmp/fetch.out
kill $server_pid
if [ -s /tmp/fetch.out ]; then
  echo "fetch_url_test passed"
else
  echo "fetch_url_test failed" >&2
  cat /tmp/fetch.out >&2
  exit 1
fi

./parseurl_test > /tmp/parseurl.out
if grep -q '^ok' /tmp/parseurl.out; then
  echo "parseurl_test passed"
else
  echo "parseurl_test failed" >&2
  cat /tmp/parseurl.out >&2
  exit 1
fi

exit 0
