using System;
using static System.Console;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Test01_Hello
{
    internal class Program
    {
        static void Main(string[] args)
        {
            //  Program pgm = new Program();
            // int i = pgm.Function(); // 함수명 수정
            Test01 Sub = new Test01();
            Sub.MainFunc();

        }

    }

    class Test01 //main class
    {
        public void MainFunc() // 접근 제어자 추가
        {
            var v = 100;
            int i = 10, j = 20;
            double d = 1.5, e = 3.1;
            object o = i + 1;
            WriteLine("Hello world({0},{1},{4},{5})\nMainFunction({2},{3})", i,j,d,e,o,v); // printf 함수와 동일 
            o = d + 0.5;
            WriteLine("Hello world({0},{1})\nMainFunction({2},{3},{4})", i, j, d, e,o); // printf 함수와 동일 
            while (true)
            {
                try
                {
                    WriteLine("두 개의 정수를 입력하세요");
                    string str = ReadLine(); //str = "10 20"
                    i = int.Parse(str.Split(' ')[0]);
                    j = int.Parse(str.Split(' ')[1]);
                    Console.WriteLine($"입력한 정수는 ({i},{j})입니다"); // printf 함수와 동일 

                    WriteLine("두 개의 실수를 입력하세요");
                    str = ReadLine(); //str = "10.5 20.1"
                    d = double.Parse(str.Split(' ')[0]);
                    e = double.Parse(str.Split(' ')[1]);
                    Console.WriteLine($"입력한 실수는 ({d},{e})입니다");
                    // printf 함수와 동일
                    //string str = "STX" + i.ToString() + "ETX";
                    //string str1 = $"STX{i,5}ETX";
                    //WriteLine(str); 
                    //Console.WriteLine(str1);
                }
                catch (Exception e1)
                {
                    WriteLine(e1.Message);
                }
            }
           
        }
    }
}