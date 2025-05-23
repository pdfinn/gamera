#!/bin/sh
# Build and run the unit tests.
# Requires plan9port's 'mk' tool. fetch_url_test starts a local HTTP
# server on the loopback interface, so networking on localhost must be
# allowed. Set OFFLINE=1 to run with a mock fetcher and skip the server.
set -e
cd "$(dirname "$0")"

# ensure Plan 9 'mk' build tool is available
if ! command -v mk >/dev/null 2>&1; then
  echo "Error: 'mk' not found in PATH." >&2
  echo "Install plan9port and ensure mk is in your PATH (try scripts/install_deps.sh)." >&2
  exit 1
fi
mk clean >/dev/null
MKFLAGS=""
if [ -n "$OFFLINE" ]; then
  MKFLAGS="FETCHER_SRC=mock_fetcher.c"
fi
mk $MKFLAGS parser_test parseurl_test fetch_url_test html_test >/tmp/test_build.log && tail -n 20 /tmp/test_build.log

# run parser_test
./parser_test > /tmp/parser_test.out
if grep -q '^ok' /tmp/parser_test.out; then
  echo "parser_test passed"
else
  echo "parser_test failed" >&2
  cat /tmp/parser_test.out >&2
  exit 1
fi

# run fetch_url_test
if [ -n "$OFFLINE" ]; then
  ./fetch_url_test dummy >/tmp/fetch.out
else
  PORT=8888
  python3 -m http.server $PORT --directory .. >/tmp/http.log 2>&1 &
  server_pid=$!
  sleep 1
  ./fetch_url_test "http://127.0.0.1:$PORT/README.md" > /tmp/fetch.out
  kill $server_pid
fi
if grep -q '^HTTP/' /tmp/fetch.out; then
  echo "fetch_url_test failed: headers present" >&2
  cat /tmp/fetch.out >&2
  exit 1
fi
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

./html_test > /tmp/html.out
if grep -q '^ok' /tmp/html.out; then
  echo "html_test passed"
else
  echo "html_test failed" >&2
  cat /tmp/html.out >&2
  exit 1
fi

exit 0
