//useless functions

//double update_cum_sum_array_find_ESS(Vparticle * pointer_to_current_particles,std::valarray<double> & weight_cum_sum,size_t Num_of_particles){
    
    //double wi_sum=0;
    //double wi_sq_sum=0;
    //weight_cum_sum=0; //Reinitialize the cum sum array 
    //for (size_t i=0; i<Num_of_particles ;i++){
    ////for (size_t i=0; i<pointer_to_current_particles->particles.size() ;i++){
        ////update the cum sum array
        //double w_i=pointer_to_current_particles->particles[i]->weight();
        //weight_cum_sum[i+1]=weight_cum_sum[i]+w_i;
        //wi_sum = wi_sum + w_i;
        ////wi_sq_sum = wi_sq_sum + pow(pointer_to_current_particles->particles[i]->weight,2);
        //wi_sq_sum = wi_sq_sum + w_i * w_i;
        ////cout<<weight_cum_sum[i]<<endl;
    //}
    ////check for the cum weight
    //dout << "### particle weights ";for (size_t i=0;i<Num_of_particles;i++){dout<<pointer_to_current_particles->particles[i]->weight()<<"  ";}dout<<std::endl<<std::endl;
    //dout << "### updated cum sum of particle weight ";for (size_t i=0;i<weight_cum_sum.size();i++){dout<<weight_cum_sum[i]<<"  ";}dout<<std::endl;
    //return wi_sum * wi_sum / wi_sq_sum;
//}

void online_reset_model_Ne(Model * model, vector <Pfcount> countContainer, vector<double>* change_times, bool online, bool print){
    if (online){
        reset_model_Ne(model, countContainer, change_times, print);
    }
    return;    
}

/*! \ingroup group_Ne */
void reset_model_Ne(Model * model, vector <Pfcount> countContainer, vector<double>* change_times, bool print){ //change countContainer to pointer
    model->resetTime(); 
    
    double total_coal_count ;
    double total_weighted_BL ;
    if (print){
    cout<<std::setw(15)<<"time" << std::setw(15)<<"count" << std::setw(15)<<"popsize" << endl;
    }
    for (size_t j=0; j < change_times->size();){
        total_coal_count=0;
        total_weighted_BL=0;
        for (size_t i=0;i<countContainer.size();i++){
            total_coal_count += countContainer[i].weightedCoalescent_[j];
            total_weighted_BL += countContainer[i].weightedBL_[j];
        }    
        double Nehat = total_weighted_BL / total_coal_count /2 ;
        
        if (print){
            //cout << "xstart = " << countContainer
            
            cout << std::setw(15) << total_weighted_BL
                 << std::setw(15) << total_coal_count
                 << std::setw(15) << Nehat<<endl;
        }
        
        dout << "Time interval " << j+1<<" " 
        <<"total coalescent = " << total_coal_count
        <<", total BL = " << total_weighted_BL
        <<", True Ne = " << model->population_size()
        <<", Ne hat= " <<  Nehat <<endl;
        
        model->addPopulationSizes(change_times->at(j), Nehat ,false,false);
        j++;
        
        if ( j<change_times->size() ){
            model->increaseTime();
        }
    }
    model->resetTime(); 
    return ;
}


//void systemetic_resampling(std::valarray<double> cum_sum, std::valarray<double> even_space, std::valarray<int> & sample_count){
    //cum_sum=cum_sum/cum_sum[cum_sum.size()-1];
    //double u=unifRand()/sample_count.size();
    //size_t interval_j=0;
    //size_t sample_i=0;
    //double uj=0;

    //dout<<std::endl<<"systematic sampling procedue" << std::endl;
    //for (size_t i=0;i<cum_sum.size();i++){dout<<cum_sum[i]<<"  ";}dout<<std::endl;
    
    //while ((interval_j<sample_count.size()) && (sample_i<sample_count.size())){//remove the second condition...
        //uj=u+even_space[sample_i];
        //dout << "Is " <<  uj<<" in the interval of " << std::setw(10)<<cum_sum[interval_j]<< " and " << std::setw(10)<< cum_sum[interval_j+1];
        //if ( (cum_sum[interval_j]<uj) && (uj<cum_sum[interval_j+1])){
            //sample_count[interval_j]++;
            //sample_i++;
            //dout << "  yes, update sample count of particle " << interval_j<<" to " << sample_count[interval_j] <<std::endl;
        //}
        //else{
            //interval_j++;
            //dout << "   no, try next interval " << std::endl;
        //}
    //}
    //dout << "systematic sampling procedue finished with total sample count " << sample_count.sum()<<std::endl<<std::endl;

//}



void check_copy_forest(Vparticle * pointer_to_current_particles,Model* model, RandomGenerator* rg,size_t Num_of_particles){
        dout << " --------------------    Initial Particles   --------------------" << std::endl;
        
        Aparticle *  new_particle = new Aparticle(model,rg);
        pointer_to_current_particles->push(new_particle);
    pointer_to_current_particles->particles[0]->particle_forest->sampleNextGenealogy();
    pointer_to_current_particles->particles[0]->particle_forest->sampleNextGenealogy();
    pointer_to_current_particles->particles[0]->particle_forest->printTree();
    pointer_to_current_particles->particles[0]->particle_forest->printTree();

    for (size_t i=1; i<Num_of_particles ;i++){
        dout << "****************** Particle # " << i << "  ******************" << std::endl;
        
        Aparticle * new_copy_particle= new Aparticle(pointer_to_current_particles->particles[0]);
        pointer_to_current_particles->push(new_copy_particle);
        pointer_to_current_particles->particles[i]->particle_forest->printTree();
        //even_space[i]=double(i)/double(Num_of_particles); // used in resampling later
    }
    for (size_t i=0; i<Num_of_particles ;i++){
        dout << "****************** Particle # " << i << "  ******************" << std::endl;
        
        pointer_to_current_particles->particles[i]->particle_forest->sampleNextGenealogy();
        //pointer_to_current_particles->particles[i]->particle_forest.printTree();
        //even_space[i]=double(i)/double(Num_of_particles); // used in resampling later
    }    

    //Vparticle* pointer_to_current_particles=&init_p_array;
    pointer_to_current_particles->makeHeap_();
    dout<<std::endl<<"Initial particles of " << Num_of_particles<<"  were built " << std::endl;        
    dout << " --------------------    Initial Particles End  --------------------" << std::endl<<std::endl;
            
    dout << "  Check the order of the particles, the particle with the minimal sequence length must be the first!!!  " << endl;
    //check particles order, the particle with the minimal sequence length must be the first!!!
    pointer_to_current_particles->check_particle_orders();
    
}


//pfARG::Vparticle * updating_current_particles(pfARG::Vparticle * pointer_to_current_particles, valarray<int> & sample_count, valarray<double> & weight_cum_sum, double curly_L, size_t Num_of_particles){
void updating_current_particles(pfARG::Vparticle * pointer_to_current_particles, valarray<int> & sample_count, valarray<double> & weight_cum_sum, double curly_L, size_t Num_of_particles){
////void updating_current_particles(pfARG::Vparticle * pointer_to_current_particles, valarray<int> & sample_count, valarray<double> & weight_cum_sum, valarray<double> & even_space, double curly_L, size_t Num_of_particles){
    //set the current site for the simulation, and sample the new genealogy

    Vparticle * vector_of_new_particles=new Vparticle(pointer_to_current_particles, sample_count);       
    //call delete and remove the particles that are not resampled....

    pointer_to_current_particles->particles.clear();
    //delete pointer_to_current_particles;// THIS IS NOT WORKING!!! introduces more errors???!!! Check for the destructor of forest!!!
    pointer_to_current_particles=NULL;          
    pointer_to_current_particles=vector_of_new_particles;
    //vector_of_new_particles=NULL;    
    //making heap and the minimal 
    pointer_to_current_particles->makeHeap_();
    //checking for heap update
    pointer_to_current_particles->check_particle_orders();
    ////cout<<vector_of_new_particles->particles.size()<<"  " << pointer_to_current_particles->particles.size()<<endl;
    
    //delete pointer_to_current_particles;
    
    //vector_of_new_particles->makeHeap_();
    //vector_of_new_particles->check_particle_orders();
    
    //return vector_of_new_particles;
}


void Aparticle::record_coalevent(const TimeInterval & current_event, double end_time){
    this->coaleventContainer.back()->set_num_coal(1);
    this->coaleventContainer.back()->set_end_height(end_time);
    if (current_event.numberOfContemporaries()>1){
        this->coaleventContainer.back()->set_num_lineages(current_event.numberOfContemporaries());
    }
    else{
        this->coaleventContainer.back()->set_num_lineages(1);
    }
    dout << "* * * * Events recorder is called start at " << this->coaleventContainer.back()->start_height()
    <<" with " << this->coaleventContainer.back()->num_lineages() <<" lineages"
    <<", with " << this->coaleventContainer.back()->num_coal()<<" coalescent at "
    <<this->coaleventContainer.back()->end_height()<<endl;
}

void Aparticle::record_recombevent(const TimeInterval & current_event, double end_time){
    this->coaleventContainer.back()->set_num_recomb(1);
    this->coaleventContainer.back()->set_end_height(end_time);
    if (current_event.numberOfContemporaries()>1){
        this->coaleventContainer.back()->set_num_lineages(current_event.numberOfContemporaries());
    }
    else{
        this->coaleventContainer.back()->set_num_lineages(1);
    }
    dout << "* * * * Events recorder is called start at " << this->coaleventContainer.back()->start_height()
    <<" with " << this->coaleventContainer.back()->num_lineages() <<" lineages"
    <<", with " << this->coaleventContainer.back()->num_recomb()<<" recombination at "
    <<this->coaleventContainer.back()->end_height()<<endl;

}

void Aparticle::record_migevent(const TimeInterval & current_event, double end_time){
    this->coaleventContainer.back()->set_num_mig(1);
    this->coaleventContainer.back()->set_end_height(end_time);
    if (current_event.numberOfContemporaries()>1){
        this->coaleventContainer.back()->set_num_lineages(current_event.numberOfContemporaries());
    }
    else{
        this->coaleventContainer.back()->set_num_lineages(1);
    }
    dout << "* * * * Events recorder is called start at " << this->coaleventContainer.back()->start_height()
    <<" with " << this->coaleventContainer.back()->num_lineages() <<" lineages"
    <<", with " << this->coaleventContainer.back()->num_mig()<<" migration at "
    <<this->coaleventContainer.back()->end_height()<<endl;

}


double Aparticle::cal_sumofiTi() const{
    double sum_of_iTi = 0;
    //cout << "sum_of_iTi= ";
    for (size_t i = 0; i < this->coaleventContainer.size() ; i++){
        sum_of_iTi = sum_of_iTi +  coaleventContainer[i]->num_lineages() * coaleventContainer[i]->length() ;
        //cout<<sum_of_iTi<<" ";
    }
    //cout<<endl;
    //if (sum_of_iTi==0){
        //cout<<endl<<"event size =" << this->events.size()<<". ";
        //for (size_t i = 0; i < this->events.size() ; i++){
        //cout<<i<<" " << events[i]->numberOfContemporaries()<<"*(" <<  events[i]->end_height()<<" - " <<  events[i]->start_height()<<")=" << events[i]->numberOfContemporaries() * ( events[i]->end_height() - events[i]->start_height() )<<", ";
        //}
        ////cout<<endl;
        //cout<<sum_of_iTi<<endl;
    //}
    return sum_of_iTi;
} 


/*! Clear coalescent events recorded when the initial particles are constructed. */
void Aparticle::clear_initial_Coalevent(){
    this->CoaleventContainer.clear();
    Coalevent* new_event = new Coalevent();
    this->CoaleventContainer.push_back(new_event);
    };
    
void pfARG::param::wright_R_script(){
    //draw TMRCA plot 
    remove(R_NAME.c_str());
    ofstream R_file;
    R_file.open (this->R_NAME.c_str(), ios::out | ios::app | ios::binary); 
    R_file<<"rm(list=ls()) \n library(stats) \n";
    R_file<<"vcf_file= \"" << this->vcf_NAME<<"\"\n vcf_data=read.table(vcf_file) \n hetro_base=vcf_data$V2[vcf_data$V7==\"PASS\"] \n";
    R_file<<"TMRCA_file= \"" << this->TMRCA_NAME<<"\"\n mydata=read.table(TMRCA_file) \n n_row=dim(mydata)[1] \n n_col=dim(mydata)[2]-1 \n ";
    R_file<<"base=mydata[,1] \n base_min=min(base) \n base_max=max(base) \n";
    //R_file<<"TMRCA=as.matrix(mydata[,-1]) \n TMRCA_mean=rowMeans(mydata[,-1]) \n TMRCA_min=min(TMRCA_mean) \n TMRCA_max=max(TMRCA_mean) \n";
    R_file<<"TMRCA=as.matrix(mydata[,-1]) \n TMRCA_mean=rowMeans(mydata[,-1]) \n TMRCA_min=min(TMRCA) \n TMRCA_max=max(TMRCA) \n";
    R_file<<"pdf(\"" << this->TMRCA_NAME<<".pdf\") \n ";
    R_file<<"plot(c(base_min,base_max),c(TMRCA_min*0.9,TMRCA_max),xlab=\"base\",ylab=\"TMRCA\",main=\"" << this->N<<" particles\",type=\"n\")\n";
//R_file<<"plot(c(base_min,base_max),c(TMRCA_min*0.9,TMRCA_max),xlab=\"base\",ylab=\"TMRCA\",main=\" << " << this->nsam<<" sample " << this->N<<" particles " << \",type=\'n\')\n";
    R_file<<"lines(base, TMRCA_mean, type=\"S\") \n ";
//#par(cin=10)
    //R_file<<"for (i in 1:(n_row-1)){ \n \t ME = 1.96 * sqrt(var(TMRCA[i,])/n_col) \n \t lines(c(base[i],base[i]),c(TMRCA_mean[i]+ME,TMRCA_mean[i]-ME),type=\"p\",pch=20,col=\"blue\")  \n \t lines(c(base[i],base[i]),c(TMRCA_mean[i]+ME,TMRCA_mean[i]-ME),type=\"l\",pch=20,col=\"blue\")  \n} \n";
    R_file<<"for (i in 1:(n_row-1)){ \n points(rep(base[i],n_col),TMRCA[i,],pch=\".\",col=\"blue\") \n} \n";
//#lines(c(base[i],base[i+1],base[i+1]),c(TMRCA_mean[i],TMRCA_mean[i],TMRCA_mean[i+1]), type="l")
    R_file<<"lines(hetro_base,rep(TMRCA_min*0.9,length(hetro_base)),type=\"p\",col=\"red\") \n dev.off() \n";
    
    //if (this->N < 150){
        R_file<<"pdf(\"" << this->TMRCA_NAME<<"-heat.pdf\") \n ";
        R_file<<"x=base \n y=TMRCA[,1] \n ";
        R_file<<"for (i in 2:(n_col)){ \n \t x=c(x,base) \n \t y=c(y,TMRCA[,i]) \n } \n";
        R_file<<"df=data.frame(x,y) \n k <- with(df,MASS:::kde2d(x,y)) \n filled.contour(k,color=heat.colors) \n dev.off() \n ";
    //}
    R_file.close();    
}






///*! 
 //* SET the model parameters, obtain number of samples from vcf file
 //*     if vcf file is empty, use default number of samples
 //*     if mutation rate is not given, use default mutation rate
 //*     if recombination rate is not given, use default recombination rate
 //*/ 
//void initialize_model(Model* model, 
                    //Param * scrm_para,
                    //pfARG::param pfARG_para,
                    //Vcf * VCFfile){
    
    //scrm_para->parse(*model);
    //size_t nsam = 2*VCFfile->nsam(); //overwrite the sample size according the data.
    
    //if (!VCFfile->withdata()){
        //nsam = pfARG_para.default_nsam;
        ////cout<<"scrm loci length is "<<model->loci_length()<<endl;
        //if ( model->loci_length() == 100000 && model->recombination_rate() == 0){
            //VCFfile->set_event_interval( pfARG_para.default_loci_length / 10 );
        //} else {
            //VCFfile->set_event_interval( model->loci_length() / 10 );
        //}
    //}    
    
    //if (model->sample_size() == 0) { // No subpopulation
        //model->addSampleSizes(0.0, vector<size_t>(1, nsam));
    //} 
    
    //else if ( model->sample_size() != 2*VCFfile->nsam() && VCFfile->withdata() ){ // Subpopulation, total number of samples do not match number of sample given by vcf file
            //throw std::invalid_argument("Total number of samples in subpopulation do not match number of samples in vcf file");    
    //}
    
    ////else if (nsam !=  model->sample_size()) { 
        ////cout  << "model->sample_size() = "<<model->sample_size()<<endl;
        ////cout <<"nsam = "<<nsam<<endl;        
        ////throw std::invalid_argument("No data given, and sum of samples in subpopulation not equal to the total sample size");    
    ////}
    
    ////check mutation rate and recombination rate    
    //if (model->recombination_rate() == 0){
        //model->set_recombination_rate(pfARG_para.default_recomb_rate, pfARG_para.default_loci_length);
    //}    

    //if (model->mutation_rate() == 0){
        //model->set_mutation_rate(pfARG_para.default_mut_rate);
    //}
    //model->finalize();
    //cout<<"model is finalized"<<endl;
    //return;
//}

