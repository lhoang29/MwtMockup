// TestMwt.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <vector>
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

// -------------------------------------- Example user code -------------------------------------- //

template <class Ctx>
class MWT;

template <class Ctx>
class IRecorder
{
public:
	virtual void Record(Ctx& context, u32 action, float probability, string unique_key) = 0;
};

template <class Ctx>
class IPolicy
{
public:
	virtual u32 Choose_Action(Ctx& context) = 0;
};

template <class Ctx>
class IScorer
{
public:
	virtual vector<float> Score_Actions(Ctx& context) = 0;
};

template <class Ctx>
class IExplorer
{
private:
	virtual std::pair<u32, float> Choose_Action(std::string unique_key, Ctx& context) = 0;

private:
	friend MWT<Ctx>;
};

template <class Ctx>
class EpsilonGreedyExplorer : public IExplorer<Ctx>
{
public:
	EpsilonGreedyExplorer(IPolicy<Ctx>& default_policy, float epsilon, u32 num_actions)
		: m_default_policy(default_policy), m_epsilon(epsilon), m_num_actions(num_actions)
	{ }

private:
	// Mark as private to prevent user access
	std::pair<u32, float> Choose_Action(std::string unique_key, Ctx& context)
	{
		return std::pair<u32, float>(0, .5f);
	}
private:
	IPolicy<Ctx>& m_default_policy;
	float m_epsilon;
	u32 m_num_actions;
};

template <class Ctx>
class SoftmaxExplorer : public IExplorer<Ctx>
{
public:
	SoftmaxExplorer(IScorer<Ctx>& default_scorer, float lambda, u32 num_actions)
		: m_default_scorer(default_scorer), m_lambda(lambda), m_num_actions(num_actions)
	{ }

private:
	// Mark as private to prevent user access
	std::pair<u32, float> Choose_Action(std::string unique_key, Ctx& context)
	{
		// Generates a distribution and samples from it to get an action and probability.
		return std::pair<u32, float>(0, .1f);
	}
private:
	IScorer<Ctx>& m_default_scorer;
	float m_lambda;
	u32 m_num_actions;
};

template <class Ctx>
class MWT
{
public:
	MWT(std::string app_id, IRecorder<Ctx>& recorder) : m_recorder(recorder)
	{
		// . . .
	}

	u32 Choose_Action(IExplorer<Ctx>& explorer, string unique_key, Ctx& context)
	{
		std::pair<u32, float> action_prob = explorer.Choose_Action(unique_key, context);

		u32 action = action_prob.first;
		float prob = action_prob.second;

		m_recorder.Record(context, action, prob, unique_key);

		return action;
	}

private:
	u64 m_app_id;
	IRecorder<Ctx>& m_recorder;
};

// -------------------------------------- Example user code -------------------------------------- //

class MyContext
{

};

class MyPolicy : public IPolicy<MyContext>
{
public:
	MyPolicy(/* ... */) { } // Custom constructors

	u32 Choose_Action(MyContext& context)
	{
		u32 whatever_action = 1;
		return whatever_action;
	}
private:
	// Custom variables, states, etc... here
};

class MyScorer : public IScorer<MyContext>
{
public:
	MyScorer(/* ... */) // Custom constructors
	{

	}

	vector<float> Score_Actions(MyContext& context)
	{
		return vector<float>();
	}
private:
	// Custom variables, states, etc... here
};

class MyRecorder : public IRecorder<MyContext>
{
public:
	virtual void Record(MyContext& context, u32 action, float probability, string unique_key)
	{

	}
};

int _tmain(int argc, _TCHAR* argv[])
{
	// Epislon greedy exploration
	{
		u32 num_actions = 10;
		float epsilon = 0.5f;
		MyRecorder logger;
		MyPolicy default_policy;
		MyContext context;
		EpsilonGreedyExplorer<MyContext> explorer(default_policy, epsilon, num_actions);
		MWT<MyContext> mwt("salt", logger);
		u32 action = mwt.Choose_Action(explorer, "unique_key", context);
	}
	// Softmax exploration
	{
		u32 num_actions = 10;
		float lambda = 0.1f;
		MyRecorder logger;
		MyScorer default_scorer;
		MyContext context;
		SoftmaxExplorer<MyContext> explorer(default_scorer, lambda, num_actions);
		MWT<MyContext> mwt("salt", logger);
		u32 action = mwt.Choose_Action(explorer, "unique_key", context);
	}
}
