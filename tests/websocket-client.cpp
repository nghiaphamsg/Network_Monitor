/* @brief: we provide you with a simple test to verify websocket-client code works
 *         Use a unit testing library (The Boost.Test library is a very common choice
 *         in this case, and since we are already using the Boost libraries,
 *         it is also a natural choice to make)
 */

#include "WebSocketClient.h"

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/test/unit_test.hpp>

#include <iostream>
#include <string>
#include <filesystem>

using NetworkMonitor::WebSocketClient;

BOOST_AUTO_TEST_SUITE(network_monitor);

/* Add a specific test just to make sure that the file is found */
BOOST_AUTO_TEST_CASE(cacert_pem)
{
   BOOST_CHECK(std::filesystem::exists(TESTS_CACERT_PEM));
}

BOOST_AUTO_TEST_CASE(test_class_WebSocketClient)
{
   /* Connection targets */
   const std::string url {"echo.websocket.org"};
   const std::string endpoint {"/"};
   const std::string port {"443"};
   const std::string message {"Hello WebSocket"};

   /* Always start with an I/O context object */
   boost::asio::io_context ioc {};

   /* TLS context object */
   boost::asio::ssl::context ctx {boost::asio::ssl::context::tlsv12_client};
   ctx.load_verify_file(TESTS_CACERT_PEM);

   /* The class under test */
   WebSocketClient client {url, endpoint, port, ioc, ctx};

   /* We use these flags to check that the connection, send, receive functions
      work as expected */
   bool connected {false};
   bool messageSent {false};
   bool messageReceived {false};
   bool messageMatches {false};
   bool disconnected {false};
   std::string echo {};

   /* Our own callbacks */
   auto onSend{[&messageSent](auto ec) {
      messageSent = !ec;
   }};

   auto onConnect{[&client, &connected, &onSend, &message](auto ec) {
      connected = !ec;
      if (!ec)
      {
         client.Send(message, onSend);
      }
   }};

   auto onClose{[&disconnected](auto ec) {
      disconnected = !ec;
   }};

   auto onReceive{[&client,
                   &onClose,
                   &messageReceived,
                   &messageMatches,
                  // &message](auto ec, auto received) {
                   &echo](auto ec, auto received) {
      messageReceived = !ec;
      // messageMatches = message == received;
      echo = std::move(received);
      client.Close(onClose);
   }};

   /* We must call io_context::run for asynchronous callbacks to run */
   client.Connect(onConnect, onReceive);
   ioc.run();

   /* When we get here, the io_context::run function has run out of work to do */
#if 0
   bool ok{
      connected &&
      messageSent &&
      messageReceived &&
      messageMatches &&
      disconnected};
   if (ok)
   {
      std::cout << "OK" << std::endl;
      return 0;
   }
   else
   {
      std::cerr << "Test failed" << std::endl;
      return 1;
   }   
#endif

   BOOST_CHECK(connected);
   BOOST_CHECK(messageSent);
   BOOST_CHECK(messageReceived);
   BOOST_CHECK(disconnected);
   BOOST_CHECK_EQUAL(message, echo);
}

bool CheckResponse (const std::string& response)
{
   /* We do not parse the whole message
      We only check that it contains some expected items */
   bool ok {true};
   ok &= response.find("ERROR") != std::string::npos;
   ok &= response.find("ValidationInvalidAuth") != std::string::npos;

   return ok;
}

BOOST_AUTO_TEST_CASE(test_send_stomp_frame)
{
   /* Connection targets */
   const std::string url {"ltnm.learncppthroughprojects.com"};
   const std::string endpoint {"/network-events"};
   const std::string port {"443"};

   /* STOMP frame */
   const std::string username {"fake_username"};
   const std::string password {"fake_password"};
   std::stringstream ss;
   ss << "STOMP" << std::endl
      << "accept-version:1.2" << std::endl
      << "host:transportforlondon.com" << std::endl
      << "login:" << username << std::endl
      << "passcode:" << password << std::endl
      << std::endl
      << '\0';

   const std::string message {ss.str()};

   /* Always start with an I/O context object */
   boost::asio::io_context ioc {};

   /* TLS context object */
   boost::asio::ssl::context ctx {boost::asio::ssl::context::tlsv12_client};
   ctx.load_verify_file(TESTS_CACERT_PEM);

   /* The class under test */
   WebSocketClient client {url, endpoint, port, ioc, ctx};

   /* We use these flags to check that the connection, send, receive functions
      work as expected */
   bool connected {false};
   bool messageSent {false};
   bool messageReceived {false};
   bool messageMatches {false};
   bool disconnected {false};
   std::string response {};

   /* Our own callbacks */
   auto onSend{[&messageSent](auto ec) {
      messageSent = !ec;
   }};

   auto onConnect{[&client, &connected, &onSend, &message](auto ec) {
      connected = !ec;
      if (!ec)
      {
         client.Send(message, onSend);
      }
   }};

   auto onClose{[&disconnected](auto ec) {
      disconnected = !ec;
   }};

   auto onReceive{[&client,
                   &onClose,
                   &messageReceived,
                   &messageMatches,
                   &response](auto ec, auto received) {
      messageReceived = !ec;
      response = std::move(received);
      client.Close(onClose);
   }};

   /* We must call io_context::run for asynchronous callbacks to run */
   client.Connect(onConnect, onReceive);
   ioc.run();

   /* When we get here, the io_context::run function has run out of work to do */
   BOOST_CHECK(connected);
   BOOST_CHECK(messageSent);
   BOOST_CHECK(messageReceived);
   BOOST_CHECK(disconnected);
   BOOST_CHECK(CheckResponse(response));
}

BOOST_AUTO_TEST_SUITE_END();