# -*- coding: utf-8 -*-
"""
Reactor Pattern Correctness Tests

Verify that the ReactorServer correctly implements the Reactor pattern:
- Each connection's I/O runs in its assigned SubReactor thread
- Connections are distributed across SubReactor threads (round-robin)
- I/O on a connection stays on the same thread (thread affinity)
- The event loop remains responsive under various conditions

Requires the server to be running on localhost:8080 with 4 worker threads.
"""

import socket
import time
import threading

import pytest

from conftest import (
    SERVER_HOST,
    SERVER_PORT,
    SERVER_THREAD_COUNT,
    create_connection,
    create_abrupt_disconnect_socket,
    query_thread_id,
    query_stats,
    echo_and_verify,
    recv_all,
    is_server_available,
    skip_if_no_server,
    wait_for_stats,
)

# ──────────────────────────────────────────────
# Fixtures
# ──────────────────────────────────────────────


@pytest.fixture(autouse=True, scope="session")
def ensure_server():
    """Skip all tests if the server is not running."""
    skip_if_no_server()


@pytest.fixture
def conn():
    """Create a single connected socket; auto-closed after test."""
    sock = create_connection()
    yield sock
    sock.close()


# ──────────────────────────────────────────────
# 1. Connection runs in a SubReactor thread
# ──────────────────────────────────────────────


class TestConnectionInSubThread:
    """Verify connections are handled by SubReactor worker threads, not the main thread."""

    def test_info_command_returns_thread_id(self, conn):
        """The ::info command must return a 'thread:<tid>' response."""
        tid = query_thread_id(conn)
        assert tid is not None, "::info did not return a 'thread:<tid>' response"

    def test_thread_id_is_nonempty(self, conn):
        """The thread ID must be a non-empty, non-zero string."""
        tid = query_thread_id(conn)
        assert tid is not None
        assert len(tid.strip()) > 0, "Thread ID is empty"
        # The server formats thread IDs as zero-padded integers
        assert (
            tid.strip() != "00000"
        ), "Thread ID is zero — connection may be in the main thread"


# ──────────────────────────────────────────────
# 2. Round-robin thread distribution
# ──────────────────────────────────────────────


class TestThreadDistribution:
    """Verify connections are distributed across multiple SubReactor threads."""

    def test_connections_spread_across_threads(self):
        """With 4 worker threads, multiple connections should be handled by at least 2 different threads."""
        thread_ids = set()
        connections = []

        try:
            for _ in range(SERVER_THREAD_COUNT * 2):
                sock = create_connection()
                connections.append(sock)
                tid = query_thread_id(sock)
                if tid is not None:
                    thread_ids.add(tid)
        finally:
            for sock in connections:
                sock.close()

        assert len(thread_ids) >= 2, (
            f"Only {len(thread_ids)} distinct thread(s) found for {SERVER_THREAD_COUNT * 2} connections — "
            "round-robin distribution may not be working"
        )

    def test_all_threads_utilized(self):
        """With enough connections, all SubReactor threads should be used."""
        thread_ids = set()
        connections = []

        try:
            # Create more connections than threads to ensure round-robin covers all
            for _ in range(SERVER_THREAD_COUNT * 3):
                sock = create_connection()
                connections.append(sock)
                tid = query_thread_id(sock)
                if tid is not None:
                    thread_ids.add(tid)
        finally:
            for sock in connections:
                sock.close()

        # On some platforms, thread scheduling may not be perfectly uniform,
        # so we accept seeing at least 3 out of 4 threads
        assert len(thread_ids) >= SERVER_THREAD_COUNT - 1, (
            f"Only {len(thread_ids)} distinct threads found — expected at least {SERVER_THREAD_COUNT - 1} "
            f"for {SERVER_THREAD_COUNT} worker threads"
        )


# ──────────────────────────────────────────────
# 3. Thread affinity per connection
# ──────────────────────────────────────────────


class TestThreadAffinity:
    """Verify that I/O on a single connection always runs on the same SubReactor thread."""

    def test_same_thread_across_multiple_queries(self, conn):
        """Sending ::info multiple times on the same connection must return the same thread ID."""
        thread_ids = set()
        for _ in range(5):
            tid = query_thread_id(conn)
            if tid is not None:
                thread_ids.add(tid)

        assert len(thread_ids) == 1, (
            f"Thread affinity violated: got {len(thread_ids)} different thread IDs "
            f"on the same connection: {thread_ids}"
        )

    def test_echo_and_info_on_same_thread(self, conn):
        """Echo operations and ::info should run on the same thread (same SubReactor)."""
        # Get thread ID before echo
        tid_before = query_thread_id(conn)
        assert tid_before is not None

        # Do some echo operations
        for i in range(3):
            assert echo_and_verify(conn, f"affinity-test-{i}")

        # Get thread ID after echo
        tid_after = query_thread_id(conn)
        assert tid_after is not None

        assert (
            tid_before == tid_after
        ), f"Thread changed during connection lifetime: {tid_before} → {tid_after}"


# ──────────────────────────────────────────────
# 4. Concurrent thread isolation
# ──────────────────────────────────────────────


class TestConcurrentThreadIsolation:
    """Verify that concurrent connections maintain correct thread assignment."""

    def test_concurrent_connections_thread_consistency(self):
        """Each concurrent connection should see a consistent thread ID across queries."""
        num_clients = SERVER_THREAD_COUNT * 2
        results = [None] * num_clients
        threads = []

        def client_worker(client_id):
            try:
                with create_connection() as sock:
                    tids = set()
                    for _ in range(3):
                        tid = query_thread_id(sock)
                        if tid is not None:
                            tids.add(tid)
                    results[client_id] = tids
            except Exception:
                results[client_id] = None

        for i in range(num_clients):
            t = threading.Thread(target=client_worker, args=(i,))
            threads.append(t)
            t.start()

        for t in threads:
            t.join(timeout=30)

        # Each client should have seen exactly 1 thread ID
        inconsistent = [i for i, r in enumerate(results) if r is None or len(r) != 1]
        assert len(inconsistent) == 0, (
            f"{len(inconsistent)} clients saw inconsistent thread IDs: "
            f"clients {inconsistent} got {[results[i] for i in inconsistent]}"
        )

    def test_concurrent_echo_correctness(self):
        """Concurrent echo operations on different connections should not mix up data."""
        num_clients = 8
        results = [None] * num_clients
        threads = []

        def echo_client(client_id):
            try:
                with create_connection() as sock:
                    msg = f"client-{client_id}-payload"
                    sock.sendall(msg.encode("utf-8"))
                    response = sock.recv(4096).decode("utf-8")
                    results[client_id] = (msg, response)
            except Exception as e:
                results[client_id] = (f"client-{client_id}-payload", f"ERROR: {e}")

        for i in range(num_clients):
            t = threading.Thread(target=echo_client, args=(i,))
            threads.append(t)
            t.start()

        for t in threads:
            t.join(timeout=30)

        mismatches = [i for i, r in enumerate(results) if r is None or r[0] != r[1]]
        assert len(mismatches) == 0, (
            f"{len(mismatches)} clients received wrong echo: "
            f"{[(i, results[i]) for i in mismatches]}"
        )


# ──────────────────────────────────────────────
# 5. Protocol interleaving
# ──────────────────────────────────────────────


class TestProtocolInterleaving:
    """Verify that ::info command doesn't break the echo protocol."""

    def test_echo_then_info_then_echo(self, conn):
        """Normal echo → ::info → normal echo should all work on the same connection."""
        # Echo
        assert echo_and_verify(conn, "before-info")

        # Info
        tid = query_thread_id(conn)
        assert tid is not None, "::info failed after echo"

        # Echo again
        assert echo_and_verify(conn, "after-info")

    def test_info_does_not_echo(self, conn):
        """The ::info command should NOT echo the '::info' string back."""
        conn.sendall(b"::info")
        time.sleep(0.1)
        response = conn.recv(1024).decode("utf-8").strip()

        assert not response.startswith(
            "::info"
        ), "::info command was echoed back instead of being handled as a protocol command"
        assert response.startswith(
            "thread:"
        ), f"Expected 'thread:<tid>' response, got: {response}"


# ──────────────────────────────────────────────
# 6. Binary data echo
# ──────────────────────────────────────────────


class TestBinaryData:
    """Verify the server correctly echoes binary (non-UTF-8) data."""

    @pytest.mark.parametrize(
        "data",
        [
            bytes(range(256)),  # All byte values 0x00-0xFF
            b"\x00\x01\x02\x03",  # Null bytes
            b"\xff\xfe\xfd\xfc",  # High byte values
            b"\x00" * 100,  # Lots of nulls
        ],
    )
    def test_binary_echo(self, conn, data):
        """Binary data should be echoed byte-for-byte."""
        conn.sendall(data)
        response = recv_all(conn, len(data))
        assert response == data, (
            f"Binary echo mismatch: sent {len(data)} bytes, "
            f"got {len(response)} bytes, "
            f"first difference at byte {next((i for i in range(min(len(data), len(response))) if data[i] != response[i]), 'end')}"
        )


# ──────────────────────────────────────────────
# 7. Idle connection resilience
# ──────────────────────────────────────────────


class TestIdleConnection:
    """Verify the server handles idle connections correctly."""

    def test_info_after_idle(self, conn):
        """After 2 seconds of idle, the connection should still respond."""
        time.sleep(2)
        tid = query_thread_id(conn)
        assert tid is not None, "Server did not respond to ::info after idle period"

    def test_echo_after_idle(self, conn):
        """Echo should work after an idle period."""
        time.sleep(2)
        assert echo_and_verify(conn, "post-idle-echo")


# ──────────────────────────────────────────────
# 8. Connection refused (negative test)
# ──────────────────────────────────────────────


class TestConnectionErrors:
    """Verify correct error handling for connection failures."""

    def test_connection_refused_on_wrong_port(self):
        """Connecting to a port with no server should raise ConnectionRefusedError."""
        with pytest.raises(ConnectionRefusedError):
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.settimeout(3)
            try:
                sock.connect((SERVER_HOST, SERVER_PORT + 9999))
            finally:
                sock.close()


# ──────────────────────────────────────────────
# 9. Round-robin determinism
# ──────────────────────────────────────────────


class TestRoundRobinDeterminism:
    """Verify the round-robin thread assignment wraps around correctly."""

    def test_round_robin_wraps_around(self):
        """
        With 4 worker threads, connections 0 and 4 should land on the same SubReactor thread
        (round-robin: 0→thread0, 1→thread1, 2→thread2, 3→thread3, 4→thread0).
        """
        connections = []
        thread_ids = []

        try:
            for _ in range(SERVER_THREAD_COUNT + 1):
                sock = create_connection()
                connections.append(sock)
                tid = query_thread_id(sock)
                assert tid is not None, "Failed to get thread ID"
                thread_ids.append(tid)
        finally:
            for sock in connections:
                sock.close()

        # Connection 0 and connection 4 (== threadCount) should share the same thread
        assert thread_ids[0] == thread_ids[SERVER_THREAD_COUNT], (
            f"Round-robin wrap-around failed: connection 0 thread={thread_ids[0]}, "
            f"connection {SERVER_THREAD_COUNT} thread={thread_ids[SERVER_THREAD_COUNT]}"
        )


# ──────────────────────────────────────────────
# 10. Multiple connections on same thread
# ──────────────────────────────────────────────


class TestSameThreadConnections:
    """Verify that connections on the same SubReactor thread don't interfere."""

    def test_two_connections_same_thread(self):
        """
        Create enough connections to guarantee at least 2 land on the same thread,
        then verify they don't interfere with each other.
        """
        connections = []
        thread_map = {}  # tid -> [socket, ...]

        try:
            for _ in range(SERVER_THREAD_COUNT * 3):
                sock = create_connection()
                connections.append(sock)
                tid = query_thread_id(sock)
                if tid is not None:
                    thread_map.setdefault(tid, []).append(sock)

            # Find a thread with at least 2 connections
            shared_thread = None
            for tid, socks in thread_map.items():
                if len(socks) >= 2:
                    shared_thread = tid
                    break

            if shared_thread is None:
                pytest.skip("Could not find a thread with 2+ connections")

            socks = thread_map[shared_thread]
            sock_a, sock_b = socks[0], socks[1]

            # Echo on both connections simultaneously
            msg_a = "message-on-same-thread-A"
            msg_b = "message-on-same-thread-B"

            sock_a.sendall(msg_a.encode("utf-8"))
            sock_b.sendall(msg_b.encode("utf-8"))

            resp_a = sock_a.recv(4096).decode("utf-8")
            resp_b = sock_b.recv(4096).decode("utf-8")

            assert resp_a == msg_a, f"Connection A got wrong echo: {resp_a}"
            assert resp_b == msg_b, f"Connection B got wrong echo: {resp_b}"
        finally:
            for sock in connections:
                sock.close()


# ──────────────────────────────────────────────
# 11. Socket resource cleanup — normal disconnect
# ──────────────────────────────────────────────


class TestNormalDisconnectCleanup:
    """Verify sockets are properly released when clients disconnect normally."""

    def test_connection_count_after_normal_close(self):
        """After a client closes normally, the server's connection count must decrease."""
        with create_connection() as monitor:
            # Wait for monitor's onConnected to fire and get baseline
            assert wait_for_stats(monitor, 1, timeout=3.0), \
                "Monitor connection not counted"
            # baseline includes the monitor itself
            baseline = query_stats(monitor)

            # Open a new connection and wait for server to count it
            client = create_connection()
            assert wait_for_stats(monitor, baseline + 1, timeout=3.0), (
                f"Count didn't reach {baseline + 1} after connect"
            )

            # Close the client and wait for server to decrement
            client.close()
            assert wait_for_stats(monitor, baseline, timeout=3.0), (
                f"Count didn't return to {baseline} after disconnect — "
                "socket resource leak detected"
            )

    def test_multiple_normal_disconnects_count_returns_to_zero(self):
        """After all clients disconnect normally, connection count should return to baseline."""
        num_clients = 8

        with create_connection() as monitor:
            assert wait_for_stats(monitor, 1, timeout=3.0)
            baseline = query_stats(monitor)

            # Open multiple connections and wait for all to be counted
            connections = [create_connection() for _ in range(num_clients)]
            assert wait_for_stats(monitor, baseline + num_clients, timeout=3.0), (
                f"Count didn't reach {baseline + num_clients} after {num_clients} connects"
            )

            # Close all connections
            for sock in connections:
                sock.close()

            # Wait for all disconnects to be processed
            assert wait_for_stats(monitor, baseline, timeout=5.0), (
                f"Count didn't return to {baseline} after all disconnects — "
                "resource leak detected"
            )

    def test_incremental_normal_disconnect(self):
        """Closing connections one at a time should decrement the count correctly each time."""
        num_clients = 5

        with create_connection() as monitor:
            assert wait_for_stats(monitor, 1, timeout=3.0)
            baseline = query_stats(monitor)

            # Open connections and wait for all to be counted
            connections = [create_connection() for _ in range(num_clients)]
            assert wait_for_stats(monitor, baseline + num_clients, timeout=3.0)

            # Close one at a time, verify count decrements after each
            for i, sock in enumerate(connections):
                expected_after_close = baseline + num_clients - (i + 1)
                sock.close()

                assert wait_for_stats(monitor, expected_after_close, timeout=3.0), (
                    f"After closing connection {i + 1}/{num_clients}: "
                    f"expected count {expected_after_close}, not reached within timeout"
                )


# ──────────────────────────────────────────────
# 12. Socket resource cleanup — abrupt disconnect (RST)
# ──────────────────────────────────────────────


class TestAbruptDisconnectCleanup:
    """Verify sockets are properly released when clients disconnect abnormally (RST)."""

    def test_rst_disconnect_releases_connection(self):
        """A client that sends RST (abrupt close) should still have its connection cleaned up."""
        with create_connection() as monitor:
            assert wait_for_stats(monitor, 1, timeout=3.0)
            baseline = query_stats(monitor)

            # Create a socket that will RST on close
            sock = create_abrupt_disconnect_socket()

            # Wait for server to count it
            assert wait_for_stats(monitor, baseline + 1, timeout=3.0)

            # Abrupt close → sends RST
            sock.close()

            # Wait for server to detect RST and clean up
            assert wait_for_stats(monitor, baseline, timeout=5.0), (
                f"Count didn't return to {baseline} after RST disconnect — "
                "socket not released after abrupt client disconnect"
            )

    def test_multiple_rst_disconnects_all_cleaned(self):
        """Multiple RST disconnects should all be properly cleaned up."""
        num_clients = 6

        with create_connection() as monitor:
            assert wait_for_stats(monitor, 1, timeout=3.0)
            baseline = query_stats(monitor)

            socks = [create_abrupt_disconnect_socket() for _ in range(num_clients)]

            # Wait for all connections to be counted
            assert wait_for_stats(monitor, baseline + num_clients, timeout=3.0)

            # Abruptly close all
            for sock in socks:
                sock.close()
            socks.clear()

            # Wait for server to process all RSTs
            assert wait_for_stats(monitor, baseline, timeout=10.0), (
                f"Count didn't return to {baseline} after all RST disconnects — "
                "resource leak detected"
            )

    def test_mixed_normal_and_rst_disconnects(self):
        """A mix of normal and RST disconnects should still clean up all connections."""
        num_normal = 3
        num_rst = 3

        with create_connection() as monitor:
            assert wait_for_stats(monitor, 1, timeout=3.0)
            baseline = query_stats(monitor)

            normal_socks = [create_connection() for _ in range(num_normal)]
            rst_socks = [create_abrupt_disconnect_socket() for _ in range(num_rst)]

            # Wait for all to be counted
            assert wait_for_stats(monitor, baseline + num_normal + num_rst, timeout=3.0)

            # Close all (mix of normal and RST)
            for sock in normal_socks:
                sock.close()
            for sock in rst_socks:
                sock.close()

            # Wait for all to be cleaned up
            assert wait_for_stats(monitor, baseline, timeout=10.0), (
                f"Count didn't return to {baseline} after mixed disconnects — "
                "resource leak detected"
            )


# ──────────────────────────────────────────────
# 13. Repeated connect/disconnect cycles (leak detection)
# ──────────────────────────────────────────────


class TestRepeatedConnectDisconnect:
    """Verify no connection leaks over many connect/disconnect cycles."""

    def test_repeated_normal_cycles_no_leak(self):
        """After 20 connect/disconnect cycles, connection count should return to baseline."""
        cycles = 20

        with create_connection() as monitor:
            assert wait_for_stats(monitor, 1, timeout=3.0)
            baseline = query_stats(monitor)

            for _ in range(cycles):
                sock = create_connection()
                sock.close()

            # Wait for all disconnects to be processed
            assert wait_for_stats(monitor, baseline, timeout=5.0), (
                f"After {cycles} normal cycles: count didn't return to {baseline}. Leak detected."
            )

    def test_repeated_rst_cycles_no_leak(self):
        """After 20 RST connect/disconnect cycles, connection count should return to baseline."""
        cycles = 20

        with create_connection() as monitor:
            assert wait_for_stats(monitor, 1, timeout=3.0)
            baseline = query_stats(monitor)

            for _ in range(cycles):
                sock = create_abrupt_disconnect_socket()
                sock.close()

            # RST disconnects may take slightly longer to clean up
            assert wait_for_stats(monitor, baseline, timeout=10.0), (
                f"After {cycles} RST cycles: count didn't return to {baseline}. Leak detected."
            )

    def test_concurrent_connect_disconnect_no_leak(self):
        """Concurrent connect/disconnect should not leak connections."""
        num_rounds = 5
        clients_per_round = 4

        with create_connection() as monitor:
            assert wait_for_stats(monitor, 1, timeout=3.0)
            baseline = query_stats(monitor)

            for _ in range(num_rounds):
                socks = [create_connection() for _ in range(clients_per_round)]
                for sock in socks:
                    sock.close()

            # Final check — wait for all cleanup
            assert wait_for_stats(monitor, baseline, timeout=10.0), (
                f"After {num_rounds} rounds of {clients_per_round} concurrent clients: "
                f"count didn't return to {baseline}. Leak detected."
            )


# ──────────────────────────────────────────────
# Entry point
# ──────────────────────────────────────────────


def run_all_tests():
    """Run all Reactor pattern tests using pytest."""
    print("ReactorServer Reactor Pattern Verification")
    print(f"Target: {SERVER_HOST}:{SERVER_PORT} ({SERVER_THREAD_COUNT} worker threads)")
    print(
        "Verifying: thread affinity, round-robin distribution, concurrency isolation..."
    )
    print()

    # Check server availability before running pytest
    if not is_server_available():
        print("Server is not running. Start reactor_server first.")
        return False

    exit_code = pytest.main([__file__, "-v"])
    return exit_code == 0


if __name__ == "__main__":
    success = run_all_tests()
    exit(0 if success else 1)
