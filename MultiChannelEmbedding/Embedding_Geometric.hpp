#pragma once
#include "Model.hpp"

#pragma once
#include "Model.hpp"

inline double sign(const double& x)
{
	return x>0?+1:-1;
}

inline vec& sign(vec& x)
{
	for_each(x.begin(), x.end(), [](double& elem){elem=sign(elem);});
	return x;
}

class GeometricEmbeddingHadamard
	:public GeometricEmbeddingModel
{
public:
	GeometricEmbeddingHadamard(int dim, double alpha)
		:GeometricEmbeddingModel(dim, alpha)
	{
		;
	}

public:
	virtual double prob_triplets( const pair<pair<string, string>,string>& triplet )
	{
		vec error = embedding_entity[name_entity[triplet.first.first]]
			% embedding_entity[name_entity[triplet.first.second]]
			% embedding_relation[name_relation[triplet.second]];

		return sum(error);
	}

	virtual double train_once( const pair<pair<string, string>,string>& triplet, double factor )
	{
		vec& head = embedding_entity[name_entity[triplet.first.first]];
		vec& tail = embedding_entity[name_entity[triplet.first.second]];
		vec& relation = embedding_relation[name_relation[triplet.second]];

		pair<pair<string, string>,string> triplet_f;
		sample_false_triplet(triplet, triplet_f);
		vec& head_f = embedding_entity[name_entity[triplet_f.first.first]];
		vec& tail_f = embedding_entity[name_entity[triplet_f.first.second]];
		vec& relation_f = embedding_relation[name_relation[triplet_f.second]];

		head += alpha * tail % relation;
		tail += alpha * head % relation;
		relation += alpha * head % tail;
		head_f -= alpha * tail_f % relation_f;
		tail_f -= alpha * head_f % relation_f;
		relation_f -= alpha * head_f % tail_f;

		head = normalise(head);
		tail = normalise(tail);
		relation = normalise(relation);
		head_f = normalise(head_f);
		tail_f = normalise(tail_f);
		relation_f = normalise(relation_f);
	}
};

class TransE
	:public GeometricEmbeddingModel
{
public:
	TransE(int dim, double alpha)
		:GeometricEmbeddingModel(dim, alpha)
	{
		;
	}

public:
	virtual double prob_triplets( const pair<pair<string, string>,string>& triplet )
	{
		vec error = embedding_entity[name_entity[triplet.first.first]] 
			+ embedding_relation[name_relation[triplet.second]] 
			- embedding_entity[name_entity[triplet.first.second]];

		return - sum(abs(error));
	}

	virtual double train_once( const pair<pair<string, string>,string>& triplet, double factor )
	{
		vec& head = embedding_entity[name_entity[triplet.first.first]];
		vec& tail = embedding_entity[name_entity[triplet.first.second]];
		vec& relation = embedding_relation[name_relation[triplet.second]];

		pair<pair<string, string>,string> triplet_f;
		sample_false_triplet(triplet, triplet_f);
		vec& head_f = embedding_entity[name_entity[triplet_f.first.first]];
		vec& tail_f = embedding_entity[name_entity[triplet_f.first.second]];
		vec& relation_f = embedding_relation[name_relation[triplet_f.second]];

		head -= alpha * sign(head + relation - tail);
		tail += alpha * sign(head + relation - tail);
		relation -= alpha * sign(head + relation - tail);
		head_f += alpha * sign(head_f + relation_f - tail_f);
		tail_f -= alpha * sign(head_f + relation_f - tail_f);
		relation_f += alpha * sign(head_f + relation_f - tail_f);

		head = normalise(head);
		tail = normalise(tail);
		relation = normalise(relation);
		head_f = normalise(head_f);
		tail_f = normalise(tail_f);
		relation_f = normalise(relation_f);
	}
};

class TransG
	:public GeneralGeometricEmbeddingModel
{
public:
	TransG(int dim, double alpha)
		:GeneralGeometricEmbeddingModel(dim, alpha)
	{
		;
	}

public:
	virtual double prob_triplets( const pair<pair<string, string>,string>& triplet )
	{
		vec error = embedding_entity[name_entity[triplet.first.first]] 
		+ embedding_relation[name_relation[triplet.second]] 
		- embedding_entity[name_entity[triplet.first.second]];

		return - as_scalar(abs(error).t() * mat_relation[name_relation[triplet.second]] * abs(error));
	}

	virtual double train_once( const pair<pair<string, string>,string>& triplet, double factor )
	{
		vec& head = embedding_entity[name_entity[triplet.first.first]];
		vec& tail = embedding_entity[name_entity[triplet.first.second]];
		vec& relation = embedding_relation[name_relation[triplet.second]];

		pair<pair<string, string>,string> triplet_f;
		sample_false_triplet(triplet, triplet_f);
		vec& head_f = embedding_entity[name_entity[triplet_f.first.first]];
		vec& tail_f = embedding_entity[name_entity[triplet_f.first.second]];
		vec& relation_f = embedding_relation[name_relation[triplet_f.second]];

		head -= alpha * sign(head + relation - tail);
		tail += alpha * sign(head + relation - tail);
		relation -= alpha * sign(head + relation - tail);
		head_f += alpha * sign(head_f + relation_f - tail_f);
		tail_f -= alpha * sign(head_f + relation_f - tail_f);
		relation_f += alpha * sign(head_f + relation_f - tail_f);

		head = normalise(head);
		tail = normalise(tail);
		relation = normalise(relation);
		head_f = normalise(head_f);
		tail_f = normalise(tail_f);
		relation_f = normalise(relation_f);

		mat_relation[name_relation[triplet.second]] -= abs(head + relation - tail) * abs(head + relation - tail).t()
			- abs(head_f + relation_f - tail_f) * abs(head_f + relation_f - tail_f).t();
	}

	virtual void train(double alpha)
	{
		for_each(mat_relation.begin(), mat_relation.end(), [=](mat& elem){elem = eye(dim, dim);});
		EmbeddingModel::train(alpha);
		for_each(mat_relation.begin(), mat_relation.end(), [=](mat& elem){elem = normalise(elem);});
	}

};

class TransGMP
	:public GeometricEmbeddingModel
{
protected:
	unsigned int sampling_times;
	vec			 error;

public:
	TransGMP(int dim, double alpha, int sampling_times =2)
		:GeometricEmbeddingModel(dim, alpha), 
		sampling_times(sampling_times),
		error(dim, 1)
	{
		;
	}

public:
	virtual double prob_triplets( const pair<pair<string, string>,string>& triplet ) = 0;
	double probability_triplets( const pair<pair<string, string>,string>& triplet, vec & error)
	{
		return prob_triplets(triplet);
	}

public:
	virtual vec grad_head(const pair<pair<string, string>,string>& triplet) = 0;
	virtual vec grad_tail(const pair<pair<string, string>,string>& triplet) = 0;
	virtual vec grad_rel(const pair<pair<string, string>,string>& triplet) = 0;

	virtual double train_once( const pair<pair<string, string>,string>& triplet, double factor )
	{
		vec& head = embedding_entity[name_entity[triplet.first.first]];
		vec& tail = embedding_entity[name_entity[triplet.first.second]];
		vec& relation = embedding_relation[name_relation[triplet.second]];
		vec head_grad(dim, 1, fill::zeros);
		vec tail_grad(dim, 1, fill::zeros);
		vec relation_grad(dim, 1, fill::zeros);

		double total_normalizor = 0;
		double sign_components[8] = {1,-1,-1,1,-1,1,1,1};
		for(auto cnt=0; cnt<sampling_times; ++cnt)
		{
			for(unsigned i=0; i<8; ++i)
			{
				bool head = i & 0x001;
				bool tail = i & 0x100;
				bool relation = i & 0x010;

				pair<pair<string, string>,string> triplet_sample;
				sample_triplet(triplet, triplet_sample, head, relation, tail);

				double prob = exp(probability_triplets(triplet_sample, error));
				if (_isnan(prob))
					continue;

				total_normalizor += prob;

				if (head == false)
				{
					head_grad -= sign_components[i] * prob * grad_head(triplet_sample);
				}
				if (tail == false)
				{
					tail_grad -= sign_components[i] * prob * grad_tail(triplet_sample);
				}
				if (relation == false)
				{
					relation_grad -= sign_components[i] * prob * grad_rel(triplet_sample);
				}
			}
		}

		head_grad /= total_normalizor;
		tail_grad /= total_normalizor;
		relation_grad /= total_normalizor;

		head_grad += grad_head(triplet);
		tail_grad += grad_tail(triplet);
		relation_grad += grad_rel(triplet);

		head -= alpha * head_grad;
		tail -= alpha * tail_grad;
		relation -= alpha * relation_grad;

		head = normalise(head);
		tail = normalise(tail);
		relation = normalise(relation);
	}
};

class TransGMPE
	:public TransGMP
{
public:
	TransGMPE(int dim, double alpha, int sampling_times = 2)
		:TransGMP(dim, alpha, sampling_times)
	{
		;
	}

public:
	virtual double prob_triplets( const pair<pair<string, string>,string>& triplet )
	{
		return - sum(abs(
			embedding_entity[name_entity[triplet.first.first]]
			+ embedding_relation[name_relation[triplet.second]]
			- embedding_entity[name_entity[triplet.first.second]]));
	}

	virtual vec grad_head( const pair<pair<string, string>,string>& triplet )
	{
		return sign(embedding_entity[name_entity[triplet.first.first]]
			+ embedding_relation[name_relation[triplet.second]]
			- embedding_entity[name_entity[triplet.first.second]]);
	}

	virtual vec grad_tail( const pair<pair<string, string>,string>& triplet )
	{
		return - sign(embedding_entity[name_entity[triplet.first.first]]
			+ embedding_relation[name_relation[triplet.second]]
			- embedding_entity[name_entity[triplet.first.second]]);
	}

	virtual vec grad_rel( const pair<pair<string, string>,string>& triplet )
	{
		return sign(embedding_entity[name_entity[triplet.first.first]]
			+ embedding_relation[name_relation[triplet.second]]
			- embedding_entity[name_entity[triplet.first.second]]);
	}
};

class TransGMPCosine
	:public TransGMP
{
public:
	TransGMPCosine(int dim, double alpha, int sampling_times = 2)
		:TransGMP(dim, alpha, sampling_times)
	{
		;
	}

public:
	virtual double prob_triplets( const pair<pair<string, string>,string>& triplet )
	{
		return - as_scalar(
			embedding_entity[name_entity[triplet.first.first]].t()
			* embedding_relation[name_relation[triplet.second]]
			- embedding_entity[name_entity[triplet.first.first]].t()
			* embedding_entity[name_relation[triplet.second]]
			- embedding_relation[name_relation[triplet.second]].t()
			* embedding_entity[name_entity[triplet.first.second]]);
	}

	virtual vec grad_head( const pair<pair<string, string>,string>& triplet )
	{
		return embedding_relation[name_relation[triplet.second]]
		- embedding_entity[name_entity[triplet.first.second]];
	}

	virtual vec grad_tail( const pair<pair<string, string>,string>& triplet )
	{
		return - embedding_entity[name_entity[triplet.first.first]]
		+ embedding_relation[name_relation[triplet.second]];
	}

	virtual vec grad_rel( const pair<pair<string, string>,string>& triplet )
	{
		return embedding_entity[name_entity[triplet.first.first]]
		- embedding_entity[name_entity[triplet.first.second]];
	}
};