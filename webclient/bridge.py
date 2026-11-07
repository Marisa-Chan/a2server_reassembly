"""WebSocket <-> raw-TCP bridge for the A2 browser client prototype.

Serves the static page from public/ and exposes /ws?host=<h>&port=<p>:
a WebSocket whose binary payloads pipe 1:1 to a raw TCP connection.
Browsers cannot open raw sockets, so the browser A2 protocol code talks
through this shim to rom_hat (8000) and the game server (9992).

Run:  python webclient/bridge.py   (listens on http://127.0.0.1:8765)
"""

import asyncio
import os
import sys
import urllib.parse
from http import HTTPStatus

import websockets
from websockets.asyncio.server import serve

LISTEN_HOST = "127.0.0.1"
LISTEN_PORT = 8765
ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "public")

CONTENT_TYPES = {
    ".html": "text/html; charset=utf-8",
    ".js": "text/javascript; charset=utf-8",
    ".css": "text/css; charset=utf-8",
    ".json": "application/json",
    ".svg": "image/svg+xml",
    ".png": "image/png",
}


def serve_static(path: str):
    path = path.split("?", 1)[0]
    if path == "/":
        path = "/index.html"
    fname = os.path.normpath(os.path.join(ROOT, path.lstrip("/")))
    if not fname.startswith(ROOT) or not os.path.isfile(fname):
        return None
    with open(fname, "rb") as f:
        body = f.read()
    ext = os.path.splitext(fname)[1].lower()
    headers = [("Content-Type", CONTENT_TYPES.get(ext, "application/octet-stream"))]
    return HTTPStatus.OK, headers, body


async def process_request(connection, request):
    if request.path.startswith("/ws"):
        return None  # normal WebSocket upgrade
    try:
        static = serve_static(request.path)
    except OSError:
        static = None
    if static is None:
        return connection.respond(HTTPStatus.NOT_FOUND, "not found")
    status, headers, body = static
    response = connection.respond(status, body.decode("utf-8", "replace"))
    for name, value in headers:
        response.headers[name] = value
    return response


async def handler(ws):
    parsed = urllib.parse.urlparse(ws.request.path)
    if parsed.path != "/ws":
        await ws.close(code=1008, reason="expected /ws")
        return

    query = urllib.parse.parse_qs(parsed.query)
    try:
        host = query["host"][0]
        port = int(query["port"][0])
    except (KeyError, ValueError):
        await ws.close(code=1008, reason="need host & port params")
        return

    print(f"[bridge] ws -> {host}:{port}", flush=True)
    try:
        tcp_reader, tcp_writer = await asyncio.wait_for(
            asyncio.open_connection(host, port), timeout=5
        )
    except (OSError, asyncio.TimeoutError) as exc:
        print(f"[bridge] connect {host}:{port} failed: {exc}", flush=True)
        await ws.close(code=1011, reason=f"tcp connect failed: {exc}")
        return

    async def ws_to_tcp():
        async for message in ws:
            if isinstance(message, bytes):
                tcp_writer.write(message)
                await tcp_writer.drain()

    async def tcp_to_ws():
        try:
            while True:
                data = await tcp_reader.read(4096)
                if not data:
                    break
                await ws.send(data)
        except (ConnectionResetError, OSError):
            pass

    ws_to_tcp_task = asyncio.create_task(ws_to_tcp())
    tcp_to_ws_task = asyncio.create_task(tcp_to_ws())
    done, pending = await asyncio.wait(
        [ws_to_tcp_task, tcp_to_ws_task], return_when=asyncio.FIRST_COMPLETED
    )
    for task in pending:
        task.cancel()
    for task in done:
        exc = task.exception()
        if exc is not None:
            print(f"[bridge] {host}:{port} task error: {exc!r}", flush=True)
    tcp_writer.close()
    print(f"[bridge] {host}:{port} closed", flush=True)


async def main():
    async with serve(
        handler, LISTEN_HOST, LISTEN_PORT, process_request=process_request
    ):
        print(f"[bridge] http://{LISTEN_HOST}:{LISTEN_PORT}/ ready", flush=True)
        await asyncio.Future()


if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        sys.exit(0)
