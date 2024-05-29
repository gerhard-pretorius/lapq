The lapq library provides a C++ client API to a PostgreSQL DB server.
It is intended for C++ applications using ASIO running on Linux.
The library is experimental and currently supports only local connections.

With this library, you can write C++ applications connecting to a Postgres DB
looking like this:

```
  asio::io_service mios;

  asio::signal_set signals(mios, SIGINT, SIGTERM);
  signals.async_wait([&](const asio::error_code &, int) { mios.stop(); });

  try
  {
    auto option = util::getEnv();
    auto c = AsyncConnection::create(mios);

    c->connect(option, [&](const std::error_code &ec)
    {
      if (ec) { DBG(ec.message()); return; }

      auto rs = std::make_shared<ResultSet>();

      c->exec(
        "select 'hello'::text as abc, 2::int as one, true::boolean as xx;",
          *rs, [&c, rs](const std::error_code &ec)
        {
          auto &rset = *rs;
          if (rset.size() < 1) { DBG("size=" << rset.size()); return; }

          if (!rset[0]) { DBG(rset[0].error()); return; }

          DBG(rset[0].get<std::string>(0,0));
          DBG(rset[0].get<int>(0,1));
          DBG(rset[0].get<bool>(0,2));

          c->close([](const std::error_code &ec)
          {
            if (ec) { DBG(ec.message()); return; }
          });
        });

    });

    mios.run();
  }
  catch(const std::system_error &e) { DBG(e.what()); throw e; }


```

