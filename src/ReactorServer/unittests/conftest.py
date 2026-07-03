# -*- coding: utf-8 -*-
"""
Shared fixtures and helpers for ReactorServer tests.
"""

import socket
import struct
import time

import pytest

SERVER_HOST = "localhost"
SERVER_PORT = 8080
SERVER_THREAD_COUNT = 4  # must match main.cc: ReactorServer(port, threadCount)


@pytest.fixture
def server_config():
    """Fixture providing server configuration."""
    return {"host": SERVER_HOST, "port": SERVER_PORT}


@pytest.fixture
def test_socket(server_config):
    """Fixture providing a connected TCP socket; closed automatically after test."""
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.settimeout(10)
    sock.connect((server_config["host"], server_config["port"]))
    yield sock
    sock.close()


def create_connection(host=SERVER_HOST, port=SERVER_PORT, timeout=10):
    """Create and return a connected TCP socket. Caller must close it."""
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.settimeout(timeout)
    sock.connect((host, port))
    return sock


def _recv_line(sock):
    """
    Read from socket until a newline character is received.
    Returns the decoded line with trailing whitespace stripped.
    This avoids buffer pollution from stale data.
    """
    buf = b""
    while True:
        chunk = sock.recv(1)
        if not chunk:
            break
        buf += chunk
        if chunk == b"\n":
            break
    return buf.decode("utf-8").strip()


def query_thread_id(sock):
    """
    Send ::info command and return the thread ID string.

    Server responds with 'thread:<tid>\\n'.
    Returns the tid portion, or None on failure.
    """
    sock.sendall(b"::info")
    data = _recv_line(sock)
    if data.startswith("thread:"):
        return data[len("thread:") :]
    return None


def query_stats(sock):
    """
    Send ::stats command and return the current active connection count.

    Server responds with 'active:<N>\\n'.
    Returns the integer count, or None on failure.
    """
    sock.sendall(b"::stats")
    data = _recv_line(sock)
    if data.startswith("active:"):
        try:
            return int(data[len("active:") :])
        except ValueError:
            return None
    return None


def create_abrupt_disconnect_socket(host=SERVER_HOST, port=SERVER_PORT, timeout=10):
    """
    Create a connected socket configured to send RST on close (abrupt disconnect).

    By setting SO_LINGER with timeout=0, closing the socket sends a TCP RST
    instead of the normal FIN handshake. This simulates a client crash.
    """
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.settimeout(timeout)
    sock.setsockopt(
        socket.SOL_SOCKET, socket.SO_LINGER, struct.pack("ii", 1, 0)
    )  # linger on, timeout=0 → RST
    sock.connect((host, port))
    return sock


def echo_and_verify(sock, message):
    """Send a message and verify the echo matches exactly."""
    sock.sendall(message.encode("utf-8"))
    response = sock.recv(4096).decode("utf-8")
    return response == message


def recv_all(sock, expected_size, chunk_size=4096):
    """Receive exactly expected_size bytes from socket."""
    received = b""
    while len(received) < expected_size:
        chunk = sock.recv(min(chunk_size, expected_size - len(received)))
        if not chunk:
            break
        received += chunk
    return received


def wait_for_stats(sock, expected_count, timeout=5.0, poll_interval=0.05):
    """
    Poll query_stats on a PERSISTENT monitor connection until count matches.

    This replaces arbitrary time.sleep() with precise condition-based waiting.
    The server's onConnected/onDisconnected callbacks are asynchronous — they
    fire in the SubReactor's event loop after the TCP handshake completes, so
    create_connection() may return before the server has counted the connection.

    IMPORTANT: Use a persistent monitor socket, NOT a temporary connection.
    Opening/closing temp connections for each query causes observer effect
    (each temp connection increments/decrements the count asynchronously).

    Args:
        sock: A persistent, already-connected monitor socket.
        expected_count: The expected active connection count.
        timeout: Maximum seconds to wait (default 5s).
        poll_interval: Seconds between polls (default 50ms).

    Returns:
        True if count reached expected_count within timeout, False otherwise.
    """
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        count = query_stats(sock)
        if count == expected_count:
            return True
        time.sleep(poll_interval)
    return False


def wait_for_connection_count(
    expected_count, timeout=5.0, poll_interval=0.05,
    host=SERVER_HOST, port=SERVER_PORT
):
    """
    Poll ::stats until the active connection count reaches the expected value.

    Opens a temporary connection for each poll. The count INCLUDES the query
    socket itself (once its onConnected fires). This is suitable for simple
    baseline checks. For precise test assertions, prefer wait_for_stats()
    with a persistent monitor socket.

    Args:
        expected_count: The expected active connection count.
        timeout: Maximum seconds to wait (default 5s).
        poll_interval: Seconds between polls (default 50ms).
        host/port: Server address.

    Returns:
        True if count reached expected_count within timeout, False otherwise.
    """
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        try:
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
                sock.settimeout(3)
                sock.connect((host, port))
                count = query_stats(sock)
                if count == expected_count:
                    return True
        except (ConnectionRefusedError, socket.timeout, OSError):
            pass
        time.sleep(poll_interval)
    return False


def get_connection_count(host=SERVER_HOST, port=SERVER_PORT):
    """
    Get the current active connection count by opening a temporary connection.

    The count INCLUDES the query socket itself (once onConnected fires).
    For test assertions, prefer using a persistent monitor with wait_for_stats().

    Returns the integer count, or None on failure.
    """
    try:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
            sock.settimeout(3)
            sock.connect((host, port))
            return query_stats(sock)
    except (ConnectionRefusedError, socket.timeout, OSError):
        return None


def is_server_available(host=SERVER_HOST, port=SERVER_PORT):
    """Check if the ReactorServer is running and accepting connections."""
    try:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
            sock.settimeout(3)
            sock.connect((host, port))
            return True
    except (ConnectionRefusedError, socket.timeout, OSError):
        return False


def skip_if_no_server():
    """Return a pytest skip marker if server is not available."""
    if not is_server_available():
        pytest.skip(
            "ReactorServer is not running on {}:{} — start it first".format(
                SERVER_HOST, SERVER_PORT
            )
        )
