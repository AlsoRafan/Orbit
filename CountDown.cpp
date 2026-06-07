#include<iostream>
#include<thread>
#include<chrono>
#include<iomanip>
using namespace std;
class Timer{
	protected:
		int time;
		public:
			Timer(int t=0){
				time=t;
			}
			void setTime()
			{
				int choice,t;
				cout<<"Choose Time:"<<endl;
				cout<<"1. in seconds:\n";
				cout<<"2. in hours:\n";
				cout<<"3. in mintues:\n";
				cout<<"Enter your choice:";
				cin>>choice;
				cout<<"Enter time:";
				cin>>t;
				switch(choice){
					case 1:
						time=t;
						break;
						case 2:
							time=t*3600;
							break;
							case 3:
								time=t*60;
								break;
								default:
									cout<<"Invalid time!"<<endl;
				}
			}
			virtual void start()=0;
};
class CountDown:public Timer{
	public:
	CountDown(int t=0):Timer(t){
	}
	/*void Display(){
		cout<<time<<endl;*/
		void Display(){
        int hours=time/3600;
        int minutes=(time%3600)/60;
        int seconds=time%60;
        cout<<setw(2)<<setfill('0')<<hours<<":"
            <<setw(2)<<setfill('0')<<minutes<< ":"
            <<setw(2)<<setfill('0')<<seconds<< "\r";
            //cout<<hours<<" : "<<minutes<<" : "<<seconds<<endl;
	}
	void start(){
		while(time>0){
			Display();
			this_thread::sleep_for(chrono::seconds(1));
			time--;
		}
		cout<<"Time's up!"<<endl;
	}
};
int main(){
	Timer*t;
	CountDown c;
	t=&c;
	t->setTime();
	t->start();
}
