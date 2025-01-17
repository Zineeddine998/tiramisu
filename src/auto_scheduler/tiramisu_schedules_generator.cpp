#include <tiramisu/auto_scheduler/schedules_generator.h>
#include <tiramisu/auto_scheduler/evaluator.h>

namespace tiramisu::auto_scheduler
{

std::vector<syntax_tree*> exhaustive_generator::generate_schedules(syntax_tree const& ast, optimization_type optim)
{
    std::vector<syntax_tree*> states;
    
    switch(optim)
    {
        case optimization_type::FUSION:
            generate_fusions(ast.roots, states, ast);
            break;

        case optimization_type::TILING:
            for (ast_node *root : ast.roots)
                generate_tilings(root, states, ast);
            
            break;

        case optimization_type::INTERCHANGE:
            for (ast_node *root : ast.roots)
                generate_interchanges(root, states, ast);
                    
            break;

        case optimization_type::UNROLLING:
            for (ast_node *root : ast.roots)
                generate_unrollings(root, states, ast);
                    
            break;

        default:
            break;
    }
    
    return states;
}

void exhaustive_generator::generate_fusions(std::vector<ast_node*> const& tree_level, std::vector<syntax_tree*>& states, syntax_tree const& ast)
{
    for (int i = 0; i < tree_level.size(); ++i)
    {
        if (tree_level[i]->unrolled || tree_level[i]->get_extent() <= 1)
            continue;

        for (int j = i + 1; j < tree_level.size(); ++j)
        {
            if (tree_level[j]->unrolled || tree_level[j]->get_extent() <= 1)
                continue;

            if (tree_level[i]->name == tree_level[j]->name &&
                tree_level[i]->low_bound == tree_level[j]->low_bound &&
                tree_level[i]->up_bound == tree_level[j]->up_bound)
            {
                // Copy the AST, and add fusion to the list of optimizations
                syntax_tree* new_ast = new syntax_tree();
                ast_node *new_node = ast.copy_and_return_node(*new_ast, tree_level[i]);
                
                optimization_info optim_info;
                optim_info.type = optimization_type::FUSION;
                optim_info.node = new_node;
                
                optim_info.nb_l = 2;
                optim_info.l0 = i;
                optim_info.l1 = j;
                new_ast->new_optims.push_back(optim_info);
                
                states.push_back(new_ast);
            }
        }
    }

    for (ast_node* node : tree_level)
        generate_fusions(node->children, states, ast);
}

void exhaustive_generator::generate_tilings(ast_node *node, std::vector<syntax_tree*>& states, syntax_tree const& ast)
{
    int branch_depth = node->get_loop_levels_chain_depth();
    
    // Generate tiling with dimension 2
    if (node->depth + 1 < branch_depth)
    {
        for (int tiling_size1 : tiling_factors_list)
        {
            if (!can_split_iterator(node->get_extent(), tiling_size1))
                continue;
                
            ast_node *node2 = node->children[0];
            for (int tiling_size2 : tiling_factors_list)
            {
                if (!can_split_iterator(node2->get_extent(), tiling_size2))
                    continue;
                    
                // Copy the AST, and add tiling to the list of optimizations
                syntax_tree *new_ast = new syntax_tree();
                ast_node *new_node = ast.copy_and_return_node(*new_ast, node);
                    
                optimization_info optim_info;
                optim_info.type = optimization_type::TILING;
                optim_info.node = new_node;
                
                optim_info.nb_l = 2;
                optim_info.l0 = node->depth;
                optim_info.l1 = node->depth + 1;
                
                optim_info.l0_fact = tiling_size1;
                optim_info.l1_fact = tiling_size2;
                
                new_node->get_all_computations(optim_info.comps);
                
                new_ast->new_optims.push_back(optim_info);
                states.push_back(new_ast);
                
                // Generate tiling with dimension 3
                if (node->depth + 2 < branch_depth)
                {
                    ast_node *node3 = node2->children[0];
                    for (int tiling_size3 : tiling_factors_list)
                    {
                        if (!can_split_iterator(node3->get_extent(), tiling_size3))
                            continue;
                            
                        // Copy the AST, and add tiling to the list of optimizations
                        syntax_tree* new_ast = new syntax_tree();
                        ast_node *new_node = ast.copy_and_return_node(*new_ast, node);
                            
                        optimization_info optim_info;
                        optim_info.type = optimization_type::TILING;
                        optim_info.node = new_node;
                        
                        optim_info.nb_l = 3;
                        optim_info.l0 = node->depth;
                        optim_info.l1 = node->depth + 1;
                        optim_info.l2 = node->depth + 2;
                        
                        optim_info.l0_fact = tiling_size1;
                        optim_info.l1_fact = tiling_size2;
                        optim_info.l2_fact = tiling_size3;
                        
                        new_node->get_all_computations(optim_info.comps);
                        
                        new_ast->new_optims.push_back(optim_info);
                        states.push_back(new_ast);
                    }
                }
            }
        }
    }
    
    for (ast_node *child : node->children)
        generate_tilings(child, states, ast);
}

void exhaustive_generator::generate_interchanges(ast_node *node, std::vector<syntax_tree*>& states, syntax_tree const& ast)
{
    if (!node->unrolled && node->get_extent() > 1)
    {
        int branch_depth = node->get_loop_levels_chain_depth();
        
        for (int i = node->depth + 1; i < branch_depth; ++i)
        {
            // Copy the AST, and add interchange to the list of optimizations
            syntax_tree* new_ast = new syntax_tree();
            ast_node *new_node = ast.copy_and_return_node(*new_ast, node);
            
            optimization_info optim_info;
            optim_info.type = optimization_type::INTERCHANGE;
            optim_info.node = new_node;
                
            optim_info.nb_l = 2;
            optim_info.l0 = node->depth;
            optim_info.l1 = i;
            new_node->get_all_computations(optim_info.comps);
                
            new_ast->new_optims.push_back(optim_info);
            states.push_back(new_ast);
        }
    }
    
    for (ast_node *child : node->children)
        generate_interchanges(child, states, ast);
}

void exhaustive_generator::generate_unrollings(ast_node *node, std::vector<syntax_tree*>& states, syntax_tree const& ast)
{
    if (!node->unrolled && node->get_extent() > 1)
    {
        for (int unrolling_factor : unrolling_factors_list)
        {
            if (node->get_extent() != unrolling_factor && 
                !can_split_iterator(node->get_extent(), unrolling_factor))
                continue;
                
            // Copy the AST, and add unrolling to the list of optimizations
            syntax_tree* new_ast = new syntax_tree();
            ast_node *new_node = ast.copy_and_return_node(*new_ast, node);

            optimization_info optim_info;
            optim_info.type = optimization_type::UNROLLING;
            optim_info.node = new_node;
                
            optim_info.nb_l = 1;
            optim_info.l0 = node->depth;
            optim_info.l0_fact = unrolling_factor;
            new_node->get_all_computations(optim_info.comps);
                
            new_ast->new_optims.push_back(optim_info);
            states.push_back(new_ast);
        }
    }
    
    for (ast_node *child : node->children)
        generate_unrollings(child, states, ast);
}

std::vector<syntax_tree*> ml_model_schedules_generator::generate_schedules(syntax_tree const& ast, optimization_type optim)
{
    // This method generates schedules applied on shared loops, so it does not
    // support ASTs with more than one root.
    if (ast.roots.size() > 1)
        return std::vector<syntax_tree*>();
        
    std::vector<syntax_tree*> states;
    ast_node *node = ast.roots[0];
    
    std::vector<int> shared_levels_extents;
    std::vector<int> innermost_extents;
    std::vector<ast_node*> innermost_nodes;

    std::vector<ast_node*> shared_nodes;
    std::vector<tiramisu::computation*> involved_computations;

    int nb_shared_iterators;

    int nb_try = 0;
    
    // Generate the specified optimization
    switch (optim)
    {
        case optimization_type::UNFUSE:
            shared_levels_extents = ast.get_shared_levels_extents();
            nb_shared_iterators = std::min((int)shared_levels_extents.size(), max_nb_iterators);
            
            // Check if we can unfuse
            if (shared_levels_extents.size() <= 1)
                return states;
                
            // Go to the first node with more than one child
            for (int i = 0; i < shared_levels_extents.size() - 1; ++i)
                node = node->children[0];
                
            // Stop if all nodes have only one child (nothing to unfuse).
            if (node->children.size() <= 1)
                return states;
            
            // Unfuse iterators
            for (int i = 0; i < nb_shared_iterators - 1; ++i)
            {
                // Copy the AST and add unfuse to the list of optimizations.
                syntax_tree* new_ast = ast.copy_ast();
                        
                optimization_info optim_info;
                optim_info.type = optimization_type::UNFUSE;
                            
                optim_info.nb_l = 1;
                optim_info.l0 = i;
                
                new_ast->new_optims.push_back(optim_info);
                states.push_back(new_ast);
            }
            
            break;
            
        case optimization_type::TILING:
            shared_levels_extents = ast.get_shared_levels_extents();
            nb_shared_iterators = std::min((int)shared_levels_extents.size(), max_nb_iterators);

            //shared nodes minus last shred node
            ast.get_shared_nodes_from_outermost(shared_nodes);
            shared_nodes.pop_back();

            // use nb try as to count if we reached last commun possible node (to disable 3layers tiling);
            nb_try = 0;
            
            for (auto& node_iterator:shared_nodes)
            {
                for (int tiling_size1 : tiling_factors_list)
                {   
                    // Check if tiling_size1 splits perfectly this iterator
                    if (can_split_iterator_sup(node_iterator->get_node_loop_extent(), tiling_size1))
                    {
                        for (int tiling_size2 : tiling_factors_list)
                        {
                            if (can_split_iterator_sup(node_iterator->children[0]->get_node_loop_extent(), tiling_size2))
                            {
                                // Copy the AST and add tiling with 2 dimensions to the list of optimizations
                                syntax_tree* new_ast = new syntax_tree();
                                ast_node *new_node = ast.copy_and_return_node(*new_ast, node_iterator);
                                
                                optimization_info optim_info;
                                optim_info.type = optimization_type::TILING;
                                optim_info.node = new_node; 
                                optim_info.nb_l = 2;
                                optim_info.l0 = node_iterator->depth;
                                optim_info.l1 = node_iterator->depth + 1;
                                optim_info.l0_fact = tiling_size1;
                                optim_info.l1_fact = tiling_size2;  
                                optim_info.comps = new_ast->computations_list;
                                new_ast->new_optims.push_back(optim_info);
                                states.push_back(new_ast);
                                    
                                // Cannot apply tiling with 3 dimensions,
                                // continue to apply tiling with 2 dimensions.
                                /*if ((nb_try + 2) >= shared_nodes.size())
                                    continue;*/
                                
                                if((nb_try + 1) < shared_nodes.size())
                                {
                                    for (int tiling_size3 : tiling_factors_list)
                                    {
                                        if (can_split_iterator_sup(node_iterator->children[0]->children[0]->get_node_loop_extent(), tiling_size3))
                                        {
                                            // Copy the AST and add tiling with 3 dimensions to the list of optimizations
                                            syntax_tree* new_ast = new syntax_tree();
                                            ast_node *new_node = ast.copy_and_return_node(*new_ast, node_iterator);
                                            
                                            optimization_info optim_info;
                                            optim_info.type = optimization_type::TILING;
                                            optim_info.node = new_node;
                                                
                                            optim_info.nb_l = 3;
                                            optim_info.l0 = node_iterator->depth;
                                            optim_info.l1 = node_iterator->depth + 1;
                                            optim_info.l2 = node_iterator->depth + 2;
                                            
                                            optim_info.l0_fact = tiling_size1;
                                            optim_info.l1_fact = tiling_size2;
                                            optim_info.l2_fact = tiling_size3;
                                                
                                            optim_info.comps = new_ast->computations_list;
                                            new_ast->new_optims.push_back(optim_info);
                                            states.push_back(new_ast);
                                            
                                        }
                                    }
                                }

                            }  
                        }

                    }
                }
                
                nb_try++;
            }
            break;

        case optimization_type::INTERCHANGE:
            
            ast.get_shared_nodes_from_outermost(shared_nodes);

            if(shared_nodes.size() > 0)
            {
                shared_nodes[0]->get_all_computations(involved_computations);
            }
            else
            {
                return states;
            }

            
            // To apply interchange, we pick all combinations of two iterators 
            // in the shared loop levels.
            for (int i = 0; i < shared_nodes.size(); ++i)
            {
                for (int j = i + 1; j < shared_nodes.size(); ++j)
                {
                    // Copy the AST and add interchange to the list of optimizations
                    syntax_tree* new_ast = new syntax_tree();
                    ast_node *new_node = ast.copy_and_return_node(*new_ast, shared_nodes[i]);
                    
                    optimization_info optim_info;
                    optim_info.type = optimization_type::INTERCHANGE;
                    optim_info.node = new_node;
                        
                    optim_info.nb_l = 2;
                    optim_info.l0 = shared_nodes[i]->depth;
                    optim_info.l1 = shared_nodes[j]->depth;
                        
                    optim_info.comps = new_ast->computations_list;
                    new_ast->new_optims.push_back(optim_info);
                    states.push_back(new_ast);
                }
                
            } 
            break;

        case optimization_type::UNROLLING:
            
            

            ast.stage_isl_states();
            
            node->get_innermost_nodes(innermost_nodes);

            std::reverse(innermost_nodes.begin(),innermost_nodes.end());
            //search for possible unrolling from the bottom loop until one is found
            // Apply all possible unrolling factors to all innermost iterators
            //test unrolling for all inner nodes until we find a valid
            for (ast_node* inner_most_node: innermost_nodes)
            {
                std::vector<tiramisu::computation*> involved_computations;
                inner_most_node->get_innermost_computations(involved_computations);

                std::vector<std::string> loop_names = involved_computations[0]->get_loop_level_names();
            
                std::string loop_name = loop_names[inner_most_node->depth];

                bool result = ast.fct->loop_unrolling_is_legal(var(loop_name),involved_computations);


                if(result) // unrollable: test all possible values
                {
                    ast.recover_isl_states();

                    for (int unrolling_fact : unrolling_factors_list)
                    {

                        if(can_split_iterator(inner_most_node->get_extent(),unrolling_fact))
                        {
                            // Copy the AST and add unrolling to the list of optimizations
                            syntax_tree* new_ast = new syntax_tree();
                            ast_node *new_node = ast.copy_and_return_node(*new_ast, inner_most_node);

                            optimization_info optim_info;
                            optim_info.type = optimization_type::UNROLLING;
                            optim_info.nb_l = 1;
                            
                            // When l0 is set to -1, unrolling is applied to all innermost levels, (1 to avoid that)
                            optim_info.l0 = new_node->depth;
                            optim_info.l0_fact = unrolling_fact;
                            // select this node
                            optim_info.node = new_node;
                            optim_info.comps = new_ast->get_innermost_computations();
                            new_ast->new_optims.push_back(optim_info);
                            states.push_back(new_ast);
                        }
    
                    }
                    ast.stage_isl_states();
                }

                nb_try++;

            }
            ast.recover_isl_states();

    
            break;

        case optimization_type::PARALLELIZE:

            //ast.print_isl_states();
            //ast.print_ast();
            
            ast.stage_isl_states();

            //for shared nodes the list of involved computations is always the same.
            // that's only the case when we compute test shared loop levels only (not always the case). 

            ast.get_shared_nodes_from_outermost(shared_nodes);

            if(shared_nodes.size() > 0)
            {
                shared_nodes[0]->get_all_computations(involved_computations);
            }
            else
            {
                return states;
            }


            for (ast_node* commun_node: shared_nodes)
            {
                std::vector<std::string> loop_names = involved_computations[0]->get_loop_level_names();
            
                std::string loop_name = loop_names[commun_node->depth];
                
                bool result = ast.fct->loop_parallelization_is_legal(var(loop_name),involved_computations);

                if(result) // unrollable: test all possible values
                {
                    ast.recover_isl_states();

                    // Copy the AST and add unrolling to the list of optimizations
                    syntax_tree* new_ast = new syntax_tree();
                    ast_node *new_node = ast.copy_and_return_node(*new_ast, commun_node);

                    optimization_info optim_info;
                    optim_info.type = optimization_type::PARALLELIZE;
                    optim_info.nb_l = 1;
                    
                    optim_info.l0 = new_node->depth;
                    optim_info.l0_fact = 0;
                    // select this node
                    optim_info.node = new_node;

                    optim_info.comps = involved_computations;
                    new_ast->new_optims.push_back(optim_info);
                    states.push_back(new_ast);
                

                    ast.stage_isl_states();
                    
                }

                nb_try++;

                if(nb_try == this->parallelism_search_depth)
                {
                    break;
                }
            }

            ast.recover_isl_states();

            break;

        case optimization_type::SKEWING:

            /* 
                optim_info.comps = new_ast->computations_list;
            }*/

            ast.stage_isl_states();
            //for shared nodes the list of involved computations is always the same.
            // that's only the case when we compute test shared loop levels only (not always the case). 
            ast.get_shared_nodes_from_outermost(shared_nodes);

            if(shared_nodes.size() > 1)
            {
                shared_nodes[0]->get_all_computations(involved_computations);
                shared_nodes.pop_back();//removes 2nd loop level, first is enough 
            }
            else
            {
                return states;
            }

            for (ast_node* commun_node: shared_nodes)
            {
                std::vector<std::string> loop_names = involved_computations[0]->get_loop_level_names();
            
                std::string loop_name = loop_names[commun_node->depth];
                std::string loop_name_inner = loop_names[commun_node->depth+1];
                
                auto result_skewing = ast.fct->skewing_local_solver(involved_computations,
                        var(loop_name),var(loop_name_inner),
                        skewing_inner_parallelism_number
                        );

                if(std::get<1>(result_skewing).size() > 0) // inner parallelism has solutions
                {
                    ast.recover_isl_states();
                    for(auto& param:std::get<1>(result_skewing))
                    {
                        // Copy the AST and add unrolling to the list of optimizations
                        syntax_tree* new_ast = new syntax_tree();
                        ast_node *new_node = ast.copy_and_return_node(*new_ast, commun_node);

                        optimization_info optim_info;
                        optim_info.type = optimization_type::SKEWING;
                        optim_info.node = new_node;

                        optim_info.nb_l = 2;
                        optim_info.l0 = new_node->depth;
                        optim_info.l1 = new_node->depth+1;
                        optim_info.l0_fact = std::get<0>(param);
                        optim_info.l1_fact = std::get<1>(param);

                        optim_info.comps = involved_computations;
                        new_ast->new_optims.push_back(optim_info);
                        states.push_back(new_ast);
                    }
                    ast.stage_isl_states();
                }

                if(std::get<0>(result_skewing).size() > 0) // outer parallelism has solutions
                {
                    ast.recover_isl_states();
                    for(auto& param:std::get<0>(result_skewing))
                    {
                        // Copy the AST and add unrolling to the list of optimizations
                        syntax_tree* new_ast = new syntax_tree();
                        ast_node *new_node = ast.copy_and_return_node(*new_ast, commun_node);

                        optimization_info optim_info;
                        optim_info.type = optimization_type::SKEWING;
                        optim_info.node = new_node;

                        optim_info.nb_l = 2;
                        optim_info.l0 = new_node->depth;
                        optim_info.l1 = new_node->depth+1;
                        optim_info.l0_fact = std::get<0>(param);
                        optim_info.l1_fact = std::get<1>(param);

                        optim_info.comps = involved_computations;
                        new_ast->new_optims.push_back(optim_info);
                        states.push_back(new_ast);
                    }
                    ast.stage_isl_states();
                }

                if(std::get<2>(result_skewing).size() > 0) // locality has solutions
                {
                    ast.recover_isl_states();
                    for(auto& param:std::get<2>(result_skewing))
                    {
                        // Copy the AST and add unrolling to the list of optimizations
                        syntax_tree* new_ast = new syntax_tree();
                        ast_node *new_node = ast.copy_and_return_node(*new_ast, commun_node);

                        optimization_info optim_info;
                        optim_info.type = optimization_type::SKEWING;
                        optim_info.node = new_node;

                        optim_info.nb_l = 2;
                        optim_info.l0 = new_node->depth;
                        optim_info.l1 = new_node->depth+1;
                        optim_info.l0_fact = std::get<0>(param);
                        optim_info.l1_fact = std::get<1>(param);

                        if((optim_info.l0 > 0) && (optim_info.l1 >0))
                        {//require loop reversal for correctness
                            optim_info.l2_fact= -1;
                        }

                        optim_info.comps = involved_computations;
                        new_ast->new_optims.push_back(optim_info);
                        states.push_back(new_ast);
                    }
                    ast.stage_isl_states();
                }


            }

            ast.recover_isl_states();
            break;

        default:
            break;
    }
    
    return states;
}

}
