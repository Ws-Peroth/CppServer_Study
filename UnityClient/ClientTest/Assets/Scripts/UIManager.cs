using System;
using System.Collections;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.IO;
using System.Threading;
using System.Threading.Tasks;
using UnityEngine;
using UnityEngine.PlayerLoop;
using UnityEngine.UI;

public class UIManager : Singleton<UIManager>
{
    [SerializeField] private Text errorText;
    public string errorString = "[ERROR LOG]";
    public string username = "";
    private string str = "[Chat Log]\n";
    public Text userNamePlaceHolder;
    [SerializeField] private Text text;

    private void Update()
    {
        errorText.text = $"[Error Log]\n{errorString}\n";
    }

    public void AddChatLog(string message)
    {
        str += $"\n{message.Replace("\0", "")}\n";
        Debug.Log($"Read Data : {message}, Total Data : {str}");
        text.text = str;
        LayoutRebuilder.ForceRebuildLayoutImmediate(text.rectTransform);
    }

    public void GetNameInput(InputField inputName)
    {
        var inputUsername = inputName.text;
        if (string.IsNullOrWhiteSpace(inputUsername))
        {
            return;
        }
        username = inputName.text;
        userNamePlaceHolder.text = $"Nick Name : {username}";
        inputName.Select();
        inputName.text = "";

    }

    public void InputEnd(InputField input)
    {
        if (string.IsNullOrWhiteSpace(input.text))
        {
            return;
        }

        NetworkManager.instance.SendData(input.text);
        input.Select();
        input.text = "";
    }
}
