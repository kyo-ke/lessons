using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using System.IO.Ports;
using System.Threading;
using UnityEngine.SceneManagement;

public class SerialHandler : MonoBehaviour
{
public delegate void SerialDataReceivedEventHandler(string message);
public event SerialDataReceivedEventHandler OnDataReceived;

public delegate void SerialDataOnClosedEventHandler(SerialHandler h);
public event SerialDataOnClosedEventHandler OnClosed;
private string portName ="\\\\.\\COM4";
public int baudRate    = 115200;

private SerialPort serialPort_;
private Thread thread_;
private bool isRunning_ = false;

public string message_;
private bool isNewMessageReceived_ = false;

void Awake()
{
Open();
        Debug.Log("OpenPort: " + SceneManager.GetActiveScene().name);
serialPort_.ReadTimeout = 10; //stops freeze when I added this.
}

void Update()
{
if (isNewMessageReceived_) {
}
}

void OnDestroy()
{
        // portをcloseする前に0を送る
        Write("0");
if (OnClosed != null)
OnClosed (this);
Close();
Debug.Log ("SerialHandler Closed");
}

private void Open()
{
serialPort_ = new SerialPort (portName, baudRate);//, Parity.None, 8, StopBits.One);
serialPort_.Open();

Debug.Log ("Serial port " + (serialPort_.IsOpen? "successfully opened":"failed to open"));

isRunning_ = true;

thread_ = new Thread(Read);
thread_.Start();
}

private void Close()
{
isRunning_ = false;

if (serialPort_ != null && serialPort_.IsOpen) {
serialPort_.Close();
serialPort_.Dispose();
}

if (thread_ != null && thread_.IsAlive) {
thread_.Join();
}

}

private void Read()
{
while (isRunning_ && serialPort_ != null && serialPort_.IsOpen) {
try {
message_ = serialPort_.ReadLine();
isNewMessageReceived_ = true;
OnDataReceived(message_);
} catch (System.Exception e) {
//Debug.LogWarning(e.Message);
}
}
}

public void Write(string message)
{
try {
serialPort_.Write(message);
} catch (System.Exception e) {
Debug.LogWarning(e.Message);
}
}
}

