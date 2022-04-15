using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using UnityEngine;
using UnityEngine.UI;

class NetworkManager : Singleton<NetworkManager>
{
    private const int PortNumber = 33221;
    private NetworkStream _networkStream;
    private TcpClient _client;
    private string _hostName;
    private string _username;

    private static string GetLocalIP()
    {
        var myIP = "";
        var host = Dns.GetHostEntry(Dns.GetHostName());

        foreach (var ip in host.AddressList)
        {
            if (ip.AddressFamily == AddressFamily.InterNetwork)
            {
                myIP = ip.ToString();
            }
        }

        return myIP;
    }

    private void Start()
    {
        // _hostName = GetLocalIP();
        _hostName = "192.168.47.223";
        print($"IP : {_hostName}");
        _username = $"USER{Guid.NewGuid().ToString().Substring(0, 4)}";
        UIManager.instance.username = _username;
        UIManager.instance.userNamePlaceHolder.text = $"Nick Name : {_username}";

        if (Application.internetReachability == NetworkReachability.NotReachable)
        {
            UIManager.instance.errorString += "Check your Internet\n";
            Debug.Log("Check your Internet");
            return;
        }

        // (1) IP 주소와 포트를 지정하고 TCP 연결
        try
        {
            _client = new TcpClient(_hostName, PortNumber);
        }
        catch (Exception e)
        {
            UIManager.instance.errorString += $"{e}\n";
            Debug.LogError(e);
            throw;
        }

        StartCoroutine(ReadData());
    }

    private IEnumerator ReadData()
    {
        while (true)
        {
            _networkStream = _client.GetStream();

            if (!_networkStream.CanRead)
            {
                continue;
            }
            
            var t = Task.Run(() =>
            {
                var readData = "";
                while (_networkStream.DataAvailable && _networkStream.CanRead)
                {
                    var buffer = new byte[1024];
                    var bytes = _networkStream.Read(buffer, 0, buffer.Length);
                    readData += Encoding.UTF8.GetString(buffer, 0, bytes);
                }

                return readData;
            });

            t.Wait();
            if (t.Result != "")
            {
                UIManager.instance.AddChatLog(t.Result + "\n\0");
            }
            
            yield return null;
        }
    }

    public void SendData(string message)
    {
        if (Application.internetReachability == NetworkReachability.NotReachable)
        {
            UIManager.instance.errorString += "Check your Internet\n";
            Debug.LogError("Check your Internet");
            return;
        }

        _client ??= new TcpClient(_hostName, 33221);

        var senderName = UIManager.instance.username;
        senderName = string.IsNullOrWhiteSpace(senderName) ? _username : senderName;

        var buff = Encoding.UTF8.GetBytes($"{senderName} : {message}" + "\0");

        try
        {
            // (2) NetworkStream을 얻어옴 
            var stream = _client.GetStream();

            // (3) 스트림에 바이트 데이타 전송
            stream.Write(buff, 0, buff.Length);

        }
        catch (Exception e)
        {
            UIManager.instance.errorString += $"{e}\n";
            Console.WriteLine(e);

            throw;
        }
    }
}