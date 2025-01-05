# HTTP Firewall

> [!WARNING]
> Your browser may be storing the cache of the website, so you need to reload the page completely. You can do this by using the key combination `Shift + F5`.

## Start http server

To test the website in mobile mode (and therefore test if it is responsive), in a Linux environment we can pull up an http server in Python using the following command:

```
python -m http.server 6969
```

> [!CAUTION]
> The command must be executed inside the folder where the `index.html` file is present.

> [!NOTE]
> Port `6969` is random, we could have also used port `8080`. To access the website from a PC you can visit the address http://127.0.0.1:6969 or http://0.0.0.0:6969.

### Allow access

Next you need to enable access via port `6969` through the firewall:

```
sudo ufw allow 6969
```

and in particular using the command:

```
$ sudo ufw status
Status: active

To                         Action      From
--                         ------      ----
8000                       ALLOW       Anywhere                  
8000 (v6)                  ALLOW       Anywhere (v6)
```

let's see that we can actually connect.

So on Linux, you need to run the `ifconfig` command selecting the network card's IP.

So we can go to the link on the phone:

```
http://<your-ip-address>:6969
```

> [!NOTE]
> For example if your IP address is `192.168.x.x` then you need to connect to the address:
> ```
> http://192.168.x.x:6969
> ```

### Disable access

To disable access to port `6969` you can use the command:

```
sudo ufw delete allow 6969
```

and in particular using the command:

```
$ sudo ufw status
Status: active
```

the firewall will block incoming connections again.

## Usage

To run the code you must use this command:

```bash
sudo python http_firewall.py
```

> [!CAUTION]
> I remind you that to view the website via mobile, you need to visit the following link:
> ```
> http://<your-ip-address>:6969
> ```
