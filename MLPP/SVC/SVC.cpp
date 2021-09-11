//
//  SVC.cpp
//
//  Created by Marc Melikyan on 10/2/20.
//

#include "SVC.hpp"
#include "LinAlg/LinAlg.hpp"
#include "Stat/Stat.hpp"
#include "Regularization/Reg.hpp"
#include "Utilities/Utilities.hpp"
#include "Cost/Cost.hpp"

#include <iostream>
#include <cmath>
#include <random>

namespace MLPP{

    SVC::SVC(std::vector<std::vector<double>> inputSet, std::vector<double> outputSet, std::string reg, double lambda, double alpha)
    : inputSet(inputSet), outputSet(outputSet), n(inputSet.size()), k(inputSet[0].size()), reg(reg), lambda(lambda), alpha(alpha)
    {
        y_hat.resize(n);

        weights = Utilities::weightInitialization(k);
        bias = Utilities::biasInitialization();
    }

    std::vector<double> SVC::modelSetTest(std::vector<std::vector<double>> X){
        return Evaluate(X);
    }

    double SVC::modelTest(std::vector<double> x){
        return Evaluate(x);
    }

    void SVC::gradientDescent(double learning_rate, int max_epoch, bool UI){
        LinAlg alg;
        Reg regularization;
        double cost_prev = 0;
        int epoch = 1;
        forwardPass();
        
        while(true){
            cost_prev = Cost(y_hat, outputSet);
                
            std::vector<double> error = alg.subtraction(y_hat, outputSet);

            // Calculating the weight gradients
            weights = alg.subtraction(weights, alg.scalarMultiply(learning_rate/n, alg.mat_vec_mult(alg.transpose(inputSet), error)));
            weights = regularization.regWeights(weights, lambda, alpha, reg);
 
            // Calculating the bias gradients
            bias -= learning_rate * alg.sum_elements(error) / n;
            forwardPass();
                
            if(UI) { 
                Utilities::CostInfo(epoch, cost_prev, Cost(y_hat, outputSet));
                Utilities::UI(weights, bias); 
            }
            epoch++;
            
            if(epoch > max_epoch) { break; }
        }
    }

    void SVC::SGD(double learning_rate, int max_epoch, bool UI){
        LinAlg alg;
        Reg regularization;
        double cost_prev = 0;
        int epoch = 1;
        
        while(true){
            std::random_device rd;
            std::default_random_engine generator(rd()); 
            std::uniform_int_distribution<int> distribution(0, int(n - 1));
            int outputIndex = distribution(generator);

            double y_hat = Evaluate(inputSet[outputIndex]);
            cost_prev = Cost({y_hat}, {outputSet[outputIndex]});

            double error = y_hat - outputSet[outputIndex];

            // Weight updation
            weights = alg.subtraction(weights, alg.scalarMultiply(learning_rate * error, inputSet[outputIndex]));
            weights = regularization.regWeights(weights, lambda, alpha, reg);
            
            // Bias updation
            bias -= learning_rate * error;

            y_hat = Evaluate({inputSet[outputIndex]});
                
            if(UI) { 
                Utilities::CostInfo(epoch, cost_prev, Cost({y_hat}, {outputSet[outputIndex]}));
                Utilities::UI(weights, bias); 
            }
            epoch++;
            
            if(epoch > max_epoch) { break; }
        }
        forwardPass();
    }

    void SVC::MBGD(double learning_rate, int max_epoch, int mini_batch_size, bool UI){
        LinAlg alg;
        Reg regularization;
        double cost_prev = 0;
        int epoch = 1;
        
        // Creating the mini-batches
        int n_mini_batch = n/mini_batch_size;
        auto [inputMiniBatches, outputMiniBatches] = Utilities::createMiniBatches(inputSet, outputSet, n_mini_batch);

        while(true){
            for(int i = 0; i < n_mini_batch; i++){
                std::vector<double> y_hat = Evaluate(inputMiniBatches[i]);
                cost_prev = Cost(y_hat, outputMiniBatches[i]);
                
                std::vector<double> error = alg.subtraction(y_hat, outputMiniBatches[i]);

                // Calculating the weight gradients
                weights = alg.subtraction(weights, alg.scalarMultiply(learning_rate/outputMiniBatches[i].size(), alg.mat_vec_mult(alg.transpose(inputMiniBatches[i]), error)));
                weights = regularization.regWeights(weights, lambda, alpha, reg);
    
                // Calculating the bias gradients
                bias -= learning_rate * alg.sum_elements(error) / outputMiniBatches[i].size();
                y_hat = Evaluate(inputMiniBatches[i]);
                    
                if(UI) { 
                    Utilities::CostInfo(epoch, cost_prev, Cost(y_hat, outputMiniBatches[i]));
                    Utilities::UI(weights, bias); 
                }
            }
            epoch++;
            if(epoch > max_epoch) { break; }
        }
        forwardPass(); 
    }

    double SVC::score(){
        Utilities util;
        return util.performance(y_hat, outputSet);
    }

    void SVC::save(std::string fileName){
         Utilities util;
         util.saveParameters(fileName, weights, bias);
     }

    double SVC::Cost(std::vector <double> y_hat, std::vector<double> y){
        Reg regularization;
        class Cost cost; 
        return cost.MSE(y_hat, y) + regularization.regTerm(weights, lambda, alpha, reg);
    }

    std::vector<double> SVC::Evaluate(std::vector<std::vector<double>> X){
        LinAlg alg;
        return alg.scalarAdd(bias, alg.mat_vec_mult(X, weights)); 
    }

    double SVC::Evaluate(std::vector<double> x){
        LinAlg alg;
        return alg.dot(weights, x) + bias;
    }

    // sign(wTx + b)
    void SVC::forwardPass(){
        y_hat = Evaluate(inputSet);
    }
}