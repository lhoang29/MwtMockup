// TestMwt.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <string>

using namespace std;

#ifdef WIN32
typedef unsigned __int64 u64;
typedef unsigned __int32 u32;
typedef unsigned __int16 u16;
typedef unsigned __int8  u8;
typedef signed __int64 i64;
typedef signed __int32 i32;
typedef signed __int16 i16;
typedef signed __int8  i8;
#else
typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t  u8;
typedef int64_t i64;
typedef int32_t i32;
typedef int16_t i16;
typedef int8_t i8;
#endif

// This interface is meant for internal use.
class IExplorer
{
private:
	virtual pair<u32, float> InvokeChooseAction(void* context, string unique_key) = 0;
};

template <class Ctx, class Plc>
struct EpsilonGreedy : public IExplorer
{
	EpsilonGreedy(float epsilon, u32 num_actions, Plc& policy) : policy_function(policy)
	{ 
		// . . .
	}

	pair<u32, float> InvokeChooseAction(void* context, string unique_key)
	{
		return Choose_Action(*((Ctx*)context), unique_key);
	}

	pair<u32, float> Choose_Action(Ctx& context, string unique_key)
	{
		u32 action = policy_function.Choose_Action(context); // Implicitly enforce Choose_Action() API on the policy
		float prob = 0.f;
		// . . .
		return pair<u32, float>(action, prob);
	}

private:
	Plc& policy_function;
};

template <class Ctx, class Plc>
struct Softmax : public IExplorer
{
	Softmax(float lambda, u32 num_actions, Plc& policy) : policy_function(policy)
	{
		// . . .
	}

	pair<u32, float> InvokeChooseAction(void* context, string unique_key)
	{
		return Choose_Action(*((Ctx*)context), unique_key);
	}

	pair<u32, float> Choose_Action(Ctx& context, string unique_key)
	{
		u32 action = policy_function.Choose_Action(context); // Implicitly enforce Choose_Action() API on the policy
		float prob = 0.f;
		// . . .
		return pair<u32, float>(action, prob);
	}

private:
	Plc& policy_function;
};

template <class Ctx, class Rec>
struct Mwt
{
	Mwt(string app_id, Rec& interaction_recorder) :
		m_app_id(app_id), m_interaction_recorder(interaction_recorder)
	{
		// . . .
	}

	u32 Choose_Action(IExplorer& explorer, Ctx& context, string unique_key)
	{
		return explorer.InvokeChooseAction(&context, unique_key);
	}

	template <class Plc>
	u32 Choose_Action(EpsilonGreedy<Ctx, Plc>& explorer, Ctx& context, string unique_key)
	{
		return Choose_Action<EpsilonGreedy<Ctx, Plc>, Ctx, Rec>(explorer, context, unique_key);
	}

	template <class Plc>
	u32 Choose_Action(Softmax<Ctx, Plc>& explorer, Ctx& context, string unique_key)
	{
		return Choose_Action<Softmax<Ctx, Plc>, Ctx, Rec>(explorer, context, unique_key);
	}

private:
	template <class Exp, class Ctx, class Rec>
	u32 Choose_Action(Exp& explorer, Ctx& context, string unique_key)
	{
		// Implicitly enforce Choose_Action() API on the policy
		pair<u32, float> action_prob = explorer.Choose_Action(context, unique_key + m_app_id);
		
		// Implicitly enforce Record() API on the context
		m_interaction_recorder.Record(context, action_prob.first, action_prob.second, unique_key);
		
		return action_prob.first;
	}

private:
	string m_app_id;
	Rec& m_interaction_recorder;
};

// Default Recorder that converts tuple into string format.
template <class Ctx>
struct StringRecorder
{
	void Record(Ctx& context, u32 action, float probability, string unique_key)
	{
		// Implicitly enforce To_String() API on the context
		m_recording.append(context.To_String());
		m_recording.append(to_string(action));
		m_recording.append(to_string(probability));
		m_recording.append(unique_key);
	}
private:
	string m_recording;
};

/******** Example user code ********/

struct MyContext
{
	u32 Choose_Action()
	{
		return 0;
	}

	string To_String()
	{
		return "";
	}
};

struct MyPolicy
{
	u32 Choose_Action(MyContext& my_context)
	{
		return 0;
	}
};

struct MyRecorder
{
	void Record(MyContext& context, u32 action, float probability, string unique_key)
	{
		// . . .
	}
};

int _tmain(int argc, _TCHAR* argv[])
{
	{
		// Epsilon Greedy exploration using default string recorder
		StringRecorder<MyContext> my_recorder;
		Mwt<MyContext, StringRecorder<MyContext>> mwt("salt", my_recorder);

		MyContext my_context;
		MyPolicy my_policy;
		EpsilonGreedy<MyContext, MyPolicy> my_explorer(0.5f, 10, my_policy);

		u32 action = mwt.Choose_Action(my_explorer, my_context, "key");
	}

	{
		// Softmax exploration using user-defined recorder
		MyRecorder my_recorder;
		Mwt<MyContext, MyRecorder> mwt("salt", my_recorder);

		MyContext my_context;
		MyPolicy my_policy;
		Softmax<MyContext, MyPolicy> my_explorer(0.5f, 10, my_policy);

		u32 action = mwt.Choose_Action<MyPolicy>(my_explorer, my_context, "key");
	}

	return 0;
}

