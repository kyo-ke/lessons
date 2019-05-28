using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System;
using UnityEngine.SceneManagement;

//こっちからモーターの制御を行う
//ルート -> モータの出力を決定
//現在の速度ベクトル、過去の速度ベクトル->モータの変化量を決定
//入力がdif(現在と過去のdif の差分を入力にするべき？) 出力が
//現在のunity 上のdif ->　arduino の出力のdifを決定
//unity上のdif の変化分を用いいるのが良さそう
//arduino上ではthrottle,roll,pitchの調整

public class turn_root : MonoBehaviour
{

    public SerialHandler serialHandler;

    private static float timeout = 0.05f;
    private float timeelapsed = 0.0f;
    public float counter = 0f;
    public Vector3 dif = new Vector3(2f, 0f, 1f);
    public Vector3 rawdif;
    public Vector3 nowpos;
    public Vector3 startpos = new Vector3(0f, 10f, 0f);

    public static float time1 = 3f;
    public static float time2 = 7f;
    public static float time3 = 15f;//回転は10秒
    public static float time4 = 25f;

    public float time2_1 = time2 * 4f / 7f + time3 * 3f / 7f;
    public float time2_2 = time2 * 3f / 7f + time3 * 4f / 7f;
    public float speed = 1.0f;
    public float speed1 = 0.05f;
    public float acceleration = 0.005f;


    // Start is called before the first frame update
    void Start()
    {
        Transform myTransform = this.transform;
        nowpos = startpos;
        myTransform.LookAt(nowpos + dif);
        serialHandler.Write("1");
    }

    // Update is called once per frame
    void Update()
    {
        if (Input.GetKey(KeyCode.Alpha0)) serialHandler.Write("0");
        
        timeelapsed += Time.deltaTime;
        counter += Time.deltaTime;
        if (timeelapsed >= timeout)
        {
            //立ち上がりの加速
            if (counter < time1)
            {
                dif = go_straight(speed - (speed - speed1) * ((time1 - counter) / time1), dif, nowpos);
                rawdif = (speed - (speed - speed1) * ((time1 - counter) / time1)) * dif;
                nowpos = nowpos + rawdif;


                //GameObject obj = (GameObject)Resources.Load("cube");
                //Instantiate(obj, nowpos, Quaternion.identity);
                serialHandler.Write("8");//

            }
            if (time1 <= counter && counter < time2)
            {
                dif = go_straight(speed, dif, nowpos);
                rawdif = speed * dif;
                nowpos = nowpos + rawdif;
                serialHandler.Write("f")


                //GameObject obj = (GameObject)Resources.Load("cube");
                //Instantiate(obj, nowpos, Quaternion.identity);
            }
            //急激にたーんしている感じがきもい
            if (time2 <= counter && counter < time3)
            {
                serialHandler.Write("3");//
                float r = 50f;
                float r_extend = 40f;
                float n_r;
                float theta;
                float span = (time3 - time2) / 3f;
                //回転半径を100 ~ 50で動かす
                if (counter < time2_1)
                {
                    n_r = r + r_extend - r_extend * ((counter - time2_1) / span);
                    theta = t_maker(n_r, speed);
                    rawdif = turn(theta, n_r, dif, nowpos);
                    dif = rawdif.normalized;
                    nowpos = nowpos + rawdif;
                    serialHandler.Write("6")

                    //GameObject obj = (GameObject)Resources.Load("cube");
                    //Instantiate(obj, nowpos, Quaternion.identity);

                }
                if (time2_1 <= counter && counter < time2_2)
                {
                    theta = t_maker(r, speed);
                    rawdif = turn(theta, r, dif, nowpos);
                    dif = rawdif.normalized;
                    nowpos = nowpos + rawdif;
                    serialHandler.Write("f")


                    //GameObject obj = (GameObject)Resources.Load("cube");
                    //Instantiate(obj, nowpos, Quaternion.identity);
                }
                if (time2_2 <= counter)
                {

                    n_r = r + r_extend * ((counter - time2_2) / span);
                    theta = t_maker(n_r, speed);
                    rawdif = turn(theta, n_r, dif, nowpos);
                    dif = rawdif.normalized;
                    nowpos = nowpos + rawdif;
                    serialHandler.Write("7")

                    //GameObject obj = (GameObject)Resources.Load("cube");
                    //Instantiate(obj, nowpos, Quaternion.identity);
                }

            }
            if (time3 <= counter && counter < time4)
            {
                serialHandler.Write("f");//
                dif = go_straight(speed, dif, nowpos);
                rawdif = speed * dif;
                nowpos = nowpos + rawdif;


                //GameObject obj = (GameObject)Resources.Load("cube");
                //Instantiate(obj, nowpos, Quaternion.identity);
            }
            if (time4 <= counter)
            {
                serialHandler.Write("5");//
                counter = 0;
                timeelapsed = 0.0f;
                dif = new Vector3(2f, 0f, 1f);
                startpos = new Vector3(0f, 10f, 0f);
                SceneManager.LoadScene("begin");
            }

            timeelapsed = 0.0f;
        }
    }
    

    //反時計回りの回転
    //verocity is t*r/2pi
    //dif の絶対値は1に固定

    private float t_maker(float r, float v)
    {
        float t = (float)(2 * Math.Asin(v / (2f * r)));
        return t;
    }

    private Vector3 turn(float t, float r, Vector3 p_dif,Vector3 npos)
    {
        float w = (float)(2f * r * Math.Sin(t / 2f));
        Vector3 p_next_dif = new Vector3((float)((p_dif.x * Math.Cos(t) - p_dif.z * Math.Sin(t))), 0.0f, (float)((p_dif.x * Math.Sin(t) + p_dif.z * Math.Cos(t)))).normalized;
        Vector3 next_dif = p_next_dif * w;
        Transform myTransform = this.transform;
        Vector3 pos = npos;
        Vector3 n_pos = pos + next_dif;
        this.transform.LookAt(n_pos);
        myTransform.position = n_pos;

        return next_dif;


    }



    private Vector3 go_straight(float v, Vector3 p_dif,Vector3 npos)
    {
        Transform myTransform = this.transform;
        Vector3 pos = npos;
        Vector3 next_pos = v * p_dif + pos;
        myTransform.LookAt(next_pos);
        myTransform.position = next_pos;

        return p_dif;
    }
}
