# -*- coding: utf-8 -*-
"""
Echo Server Test Client using pytest
Test the Reactor Multi-thread Echo Server functionality with enhanced concurrency tests
"""

import socket
import time
import threading
import pytest
import random
import string
import concurrent.futures

from conftest import (
    SERVER_HOST,
    SERVER_PORT,
    SERVER_THREAD_COUNT,
    create_connection,
    recv_all,
    skip_if_no_server,
)

# ──────────────────────────────────────────────
# Session-level server availability check
# ──────────────────────────────────────────────


@pytest.fixture(autouse=True, scope="session")
def ensure_server():
    """Skip all tests if the server is not running."""
    skip_if_no_server()


# ──────────────────────────────────────────────
# Core echo tests
# ──────────────────────────────────────────────


class TestReactorEchoServer:
    """Test class for Reactor Multi-thread Echo Server functionality"""

    @pytest.fixture
    def test_socket(self):
        """Fixture providing a connected socket."""
        sock = create_connection()
        yield sock
        sock.close()

    def test_single_echo(self, test_socket):
        """Test single echo request"""
        message = "Hello Echo Server!"

        test_socket.sendall(message.encode("utf-8"))
        data = test_socket.recv(1024)
        response = data.decode("utf-8")

        assert response == message, f"Expected '{message}', but got '{response}'"

    @pytest.mark.parametrize(
        "message",
        [
            "Test message 1",
            "Another test",
            "Short",
            "A" * 100,
            "Special chars: !@#$%^&*()",
            "Numbers: 1234567890",
            # Qt 的 readyRead() 不会为长度为0的数据触发
            # 因此跳过空消息测试，这是框架的预期行为
        ],
    )
    def test_multiple_messages(self, test_socket, message):
        """Test multiple different messages using parametrization"""
        test_socket.sendall(message.encode("utf-8"))
        response = test_socket.recv(1024).decode("utf-8")

        assert response == message, f"Expected '{message}', but got '{response}'"

    def test_persistent_connection(self, test_socket):
        """Test sending multiple messages on same connection"""
        for i in range(10):
            msg = f"Persistent message {i+1}"
            test_socket.sendall(msg.encode("utf-8"))
            response = test_socket.recv(1024).decode("utf-8")

            assert (
                response == msg
            ), f"Message {i+1}: Expected '{msg}', but got '{response}'"

    @pytest.mark.parametrize("client_id", [1, 2, 3, 4, 5])
    def test_concurrent_clients(self, client_id):
        """Test multiple concurrent clients"""

        def client_thread():
            try:
                with create_connection() as sock:
                    for i in range(3):
                        msg = f"Message {i} from client {client_id}"
                        sock.sendall(msg.encode("utf-8"))
                        response = sock.recv(1024).decode("utf-8")

                        if response != msg:
                            return False
                    return True
            except Exception:
                return False

        result = client_thread()
        assert result, f"Client {client_id} failed to get proper echo response"

    @pytest.mark.parametrize("message_size", [1024, 4096, 8192, 16384])
    def test_large_message(self, test_socket, message_size):
        """Test sending large messages of different sizes"""
        large_msg = "".join(
            random.choices(string.ascii_letters + string.digits, k=message_size)
        )

        test_socket.sendall(large_msg.encode("utf-8"))

        received = recv_all(test_socket, message_size)
        response = received.decode("utf-8")

        assert (
            response == large_msg
        ), f"Large message test failed: sent {message_size} bytes, received {len(response)} bytes"

    def test_rapid_fire_messages(self, test_socket):
        """Test sending messages in rapid succession"""
        messages = [f"Rapid message {i}" for i in range(20)]

        for msg in messages:
            test_socket.sendall(msg.encode("utf-8"))
            response = test_socket.recv(1024).decode("utf-8")
            assert response == msg, f"Rapid fire failed for: {msg}"

    def test_mixed_message_sizes(self, test_socket):
        """Test alternating between small and large messages"""
        messages = ["Small", "A" * 5000, "Tiny", "B" * 15000, "End"]

        for msg in messages:
            test_socket.sendall(msg.encode("utf-8"))

            expected_size = len(msg)
            received = recv_all(test_socket, expected_size)
            response = received.decode("utf-8")
            assert (
                response == msg
            ), f"Mixed sizes failed for message of length {len(msg)}"

    def test_simultaneous_concurrent_clients(self):
        """Test truly simultaneous clients using threads"""
        num_clients = 10
        results = [None] * num_clients
        threads = []

        def client_thread(client_id, results):
            try:
                with create_connection() as sock:
                    for i in range(5):
                        msg = f"Client {client_id} message {i}"
                        sock.sendall(msg.encode("utf-8"))
                        response = sock.recv(1024).decode("utf-8")

                        if response != msg:
                            results[client_id] = False
                            return

                        results[client_id] = True
            except Exception as e:
                print(f"Client {client_id} error: {e}")
                results[client_id] = False

        for i in range(num_clients):
            thread = threading.Thread(target=client_thread, args=(i, results))
            threads.append(thread)

        for thread in threads:
            thread.start()

        for thread in threads:
            thread.join()

        success_count = sum(1 for r in results if r)
        assert (
            success_count == num_clients
        ), f"Only {success_count}/{num_clients} clients succeeded"

    def test_back_to_back_connections(self):
        """Test creating many connections in quick succession"""
        successful_connections = 0
        total_connections = 15

        for i in range(total_connections):
            try:
                with create_connection(timeout=3) as sock:
                    msg = f"Quick connection {i}"
                    sock.sendall(msg.encode("utf-8"))
                    response = sock.recv(1024).decode("utf-8")

                    if response == msg:
                        successful_connections += 1
            except Exception:
                continue

        success_rate = successful_connections / total_connections
        assert (
            success_rate >= 0.8
        ), f"Back-to-back connection success rate too low: {success_rate:.1%}"

    def test_client_timeout_handling(self):
        """Test server behavior when client times out"""
        with create_connection() as sock:
            sock.settimeout(2)

            # Send a message but don't receive immediately
            sock.sendall("Test message".encode("utf-8"))

            # Wait a bit before receiving
            time.sleep(1)

            # Should still be able to receive
            response = sock.recv(1024).decode("utf-8")
            assert response == "Test message"

    def test_server_handles_invalid_data(self, test_socket):
        """Test how server handles potentially problematic data"""
        test_cases = [
            "\n",  # Just newline
            "\t",  # Tab
            " " * 100,  # Many spaces
        ]

        for test_data in test_cases:
            test_socket.sendall(test_data.encode("utf-8"))
            response = test_socket.recv(1024).decode("utf-8")
            assert (
                response == test_data
            ), f"Unexpected response for test data: '{response}'"

    def test_connection_storm(self):
        """Stress test with connection storm - many rapid connect/send/disconnect"""

        def storm_client(client_id):
            try:
                with create_connection(timeout=2) as sock:
                    msg = f"Storm message {client_id}"
                    sock.sendall(msg.encode("utf-8"))
                    response = sock.recv(1024).decode("utf-8")
                    return response == msg
            except Exception:
                return False

        num_storm_clients = 20
        threads = []
        results = []

        for i in range(num_storm_clients):
            thread = threading.Thread(
                target=lambda i=i: results.append(storm_client(i))
            )
            threads.append(thread)

        for i, thread in enumerate(threads):
            thread.start()
            if i % 5 == 0:
                time.sleep(0.1)

        for thread in threads:
            thread.join()

        success_rate = sum(results) / len(results)
        assert (
            success_rate >= 0.7
        ), f"Connection storm test success rate too low: {success_rate:.1%}"

    def test_long_running_connection(self):
        """Test keeping a connection open for an extended period with intermittent messages"""
        with create_connection() as sock:
            sock.settimeout(10)

            for i in range(6):
                msg = f"Long running message {i}"
                sock.sendall(msg.encode("utf-8"))
                response = sock.recv(1024).decode("utf-8")
                assert response == msg, f"Long running failed at iteration {i}"

                time.sleep(5)


# ──────────────────────────────────────────────
# High concurrency tests
# ──────────────────────────────────────────────


class TestHighConcurrency:
    """Advanced concurrency and load tests."""

    def test_high_concurrency_load(self):
        """Test high concurrent load with thread pool"""
        num_clients = 30
        messages_per_client = 5

        def run_client(client_id):
            try:
                with create_connection() as sock:
                    for i in range(messages_per_client):
                        msg = f"Load test client {client_id} msg {i}"
                        sock.sendall(msg.encode("utf-8"))
                        response = sock.recv(1024).decode("utf-8")

                        if response != msg:
                            return False
                    return True
            except Exception as e:
                print(f"High load client {client_id} failed: {e}")
                return False

        with concurrent.futures.ThreadPoolExecutor(max_workers=num_clients) as executor:
            futures = [executor.submit(run_client, i) for i in range(num_clients)]
            results = [f.result() for f in concurrent.futures.as_completed(futures)]

        success_count = sum(results)
        success_rate = success_count / num_clients
        assert (
            success_rate >= 0.9
        ), f"High concurrency load test failed: {success_count}/{num_clients} succeeded"

    def test_mixed_concurrent_operations(self):
        """Test mixed operations with different message sizes concurrently"""
        num_clients = 20

        def run_client(client_id):
            try:
                with create_connection() as sock:
                    messages = [
                        f"Short {client_id}",
                        "M" * 2000,
                        "L" * 8000,
                        f"Final {client_id}",
                    ]

                    for msg in messages:
                        sock.sendall(msg.encode("utf-8"))

                        if len(msg) > 1000:
                            received = recv_all(sock, len(msg))
                            response = received.decode("utf-8")
                        else:
                            response = sock.recv(1024).decode("utf-8")

                        if response != msg:
                            return False
                    return True
            except Exception as e:
                print(f"Mixed operations client {client_id} failed: {e}")
                return False

        threads = []
        results = [None] * num_clients

        for i in range(num_clients):
            thread = threading.Thread(
                target=lambda i=i: results.__setitem__(i, run_client(i))
            )
            threads.append(thread)
            thread.start()

        for thread in threads:
            thread.join()

        success_count = sum(1 for r in results if r)
        success_rate = success_count / num_clients
        assert (
            success_rate >= 0.85
        ), f"Mixed concurrent operations failed: {success_count}/{num_clients} succeeded"

    def test_sustained_high_load(self):
        """Test sustained high load over a period of time"""
        duration = 5
        num_active_clients = 15

        def sustained_client(client_id, stop_event):
            messages_sent = 0
            successes = 0

            try:
                with create_connection() as sock:
                    while not stop_event.is_set():
                        msg = f"Sustained client {client_id} msg {messages_sent}"
                        sock.sendall(msg.encode("utf-8"))
                        response = sock.recv(1024).decode("utf-8")

                        messages_sent += 1
                        if response == msg:
                            successes += 1

                        time.sleep(0.1)
            except Exception as e:
                print(f"Sustained client {client_id} error: {e}")

            return messages_sent, successes

        stop_event = threading.Event()
        threads = []
        results = []

        for i in range(num_active_clients):
            thread = threading.Thread(
                target=lambda i=i: results.append(sustained_client(i, stop_event))
            )
            threads.append(thread)
            thread.start()

        time.sleep(duration)
        stop_event.set()

        for thread in threads:
            thread.join()

        total_messages = sum(r[0] for r in results)
        total_successes = sum(r[1] for r in results)

        print(
            f"Sustained load: {total_messages} messages sent, {total_successes} successful"
        )

        if total_messages > 0:
            success_rate = total_successes / total_messages
            assert (
                success_rate >= 0.95
            ), f"Sustained high load success rate too low: {success_rate:.1%}"
            assert (
                total_messages >= num_active_clients * 10
            ), f"Not enough messages sent: {total_messages}"

    def test_connection_churn(self):
        """Test rapid connection and disconnection patterns"""
        num_cycles = 25
        connections_per_cycle = 8

        successful_connections = 0
        total_attempts = num_cycles * connections_per_cycle

        for cycle in range(num_cycles):
            threads = []
            cycle_results = []

            def connect_and_send(thread_id):
                try:
                    with create_connection(timeout=3) as sock:
                        msg = f"Churn cycle {cycle} thread {thread_id}"
                        sock.sendall(msg.encode("utf-8"))
                        response = sock.recv(1024).decode("utf-8")

                        if response == msg:
                            cycle_results.append(True)
                        else:
                            cycle_results.append(False)
                except Exception:
                    cycle_results.append(False)

            for i in range(connections_per_cycle):
                thread = threading.Thread(target=connect_and_send, args=(i,))
                threads.append(thread)
                thread.start()

            for thread in threads:
                thread.join()

            successful_connections += sum(cycle_results)

            if cycle < num_cycles - 1:
                time.sleep(0.2)

        success_rate = successful_connections / total_attempts
        assert (
            success_rate >= 0.8
        ), f"Connection churn test success rate too low: {success_rate:.1%}"


# ──────────────────────────────────────────────
# Server shutdown utility
# ──────────────────────────────────────────────


def send_shutdown_command(host=SERVER_HOST, port=SERVER_PORT):
    """Send shutdown command to the server"""
    try:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
            sock.settimeout(5)
            sock.connect((host, port))

            shutdown_command = "::quit"
            print(f"Sending shutdown command: '{shutdown_command}'")
            sock.sendall(shutdown_command.encode("utf-8"))

            try:
                response = sock.recv(1024).decode("utf-8")
                print(f"Server response: {response}")
            except socket.timeout:
                print("No response from server (expected during shutdown)")

            print("Shutdown command sent successfully")
            return True
    except Exception as e:
        print(f"Failed to send shutdown command: {e}")
        return False


def run_all_tests():
    """Run all tests using pytest and then send shutdown command"""
    print("Note: Make sure the Reactor Echo Server is running before starting tests!")
    print(f"Target: {SERVER_HOST}:{SERVER_PORT}")
    print(
        "This will test the reactor server's multi-threading and connection handling capabilities..."
    )

    exit_code = pytest.main([__file__, "-v"])

    print("\n" + "=" * 60)
    print("All tests completed. Sending shutdown command to server...")
    print("=" * 60)

    time.sleep(1)

    shutdown_success = send_shutdown_command()

    if shutdown_success:
        print("✓ Shutdown command sent successfully")
    else:
        print("✗ Failed to send shutdown command")

    return exit_code == 0


if __name__ == "__main__":
    success = run_all_tests()
    exit(0 if success else 1)
