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


```
# sudo apt-get install libssl-dev

CC=clang CXX=clang++ \
CXXFLAGS="-std=c++20" \
~/opt/cmake/bin/cmake -G "Unix Makefiles" \
-DCMAKE_PREFIX_PATH=$HOME/opt \
-DCMAKE_BUILD_TYPE=Release \
-DCMAKE_INSTALL_PREFIX=$HOME/install \
-DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
-DASIO_PREFIX=$HOME/opt \
-DTTMATH_PREFIX=$HOME/opt \
~/github/lapq
```


Run Postgres in a container and share the Unix domain socket
used to connect. This allows running the test programs on the host
and connect to the container. Note postgres is NOT running on the host.
The Postgres Dockerfile uses group/user postgres 70

/var/run -> /run
/var/run/postgresql/ -> /run/postgresql

This requires a matching user/group on the host

```
sudo addgroup --gid 70 --system postgres

sudo adduser --system --uid 70 --ingroup postgres \
--no-create-home --disabled-password --disabled-login \
postgres

sudo mkdir /run/postgresql
sudo chown postgres:postgres /run/postgresql
sudo chmod g+w,o-w /run/postgresql
```

```
docker pull alpine:3.19
docker pull postgres:16.3-alpine3.19

docker run --rm --name pgres --hostname pgres \
-e POSTGRES_PASSWORD=mysecret -d \
-v/run/postgresql:/run/postgresql \
postgres:16.3-alpine3.19

docker exec -ti -u root pgres bash

apk update
apk add openssl
exit

dk network inspect bridge --format 'table {{.Containers}}'

"IPv4Address": "172.17.0.2


docker exec -ti -u postgres pgres psql -h pgres

create role gerhard with login;
alter user gerhard with encrypted password 'mysecret';


docker exec -ti -u postgres pgres psql -h pgres -U gerhard -d template1

docker exec -ti -u postgres pgres bash

LD_LIBRARY_PATH+=:/mnt/src PGUSER=gerhard PGDATABASE=template1 ./t1

```




Todo
----

1. Automate tests using cmake.
2. Add 'make install' to .local.
3. Instructions for running db using Docker.
4. Use variant instead of any as a default for Record. And don't inherit from
   any.
5. Remove debug statements.
6. Use modules.
7. Use concepts.
8. Upgrade Asio.
9. Copy construct the formatter in DBResultSetType constructor.
