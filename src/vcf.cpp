/*
 * pf-ARG is short for particle filters for ancestral recombination graphs. 
 * This is a free software for demographic inference from genome data with particle filters. 
 * 
 * Copyright (C) 2013, 2014 Sha (Joe) Zhu and Gerton Lunter
 * 
 * This file is part of pf-ARG.
 * 
 * pf-ARG is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/


#include"vcf.hpp"
using namespace std;


Vcf::Vcf(string file_name, int buffer_length){ /*! Initialize by read in the vcf header file */
    /*! Initialize vcf file, search for the end of the vcf header. 
     *  Extract the first block of data ( "buffer_length" lines ) into buff
     */
    this->init(file_name, buffer_length);
    
    ifstream in_file;
    in_file.open(file_name.c_str());
    in_file.seekg (0, in_file.end);
    vcf_length_ = in_file.tellg();
    
    in_file.seekg (0, in_file.beg);
    string line;
    header_end_pos_=0;
    header_end_line=0;
    in_file.seekg (0, in_file.end);
    vcf_file_length = in_file.tellg();
    in_file.seekg (0, in_file.beg);
    if (in_file.good()){
        getline (in_file,line);
        header_end_pos_ += line.size()+1;
        while (line.size()>0 ){   
            dout << header_end_line<<"  " <<line.size() <<"  " << header_end_pos_<<"  " << line<<endl;
            if (line[0]=='#'){
                if (line[1]=='#'){
                    // read to header buffer
                    // TO COME ...
                } else {
                    check_feilds(line);
                    //break; //end of the header
                }
            }
    
            getline (in_file,line);
            
            if (line.find("PASS")!= std::string::npos){
                break;
            } // This will skip to the first line of the useable data
            
            header_end_pos_ += line.size()+1;
            header_end_line++;
        }    
    
    }
    this->end_pos_ = this->header_end_pos_;
    in_file.close();
    this->read_new_block(); // END by start reading a block
}

void Vcf::reset_VCF_to_data(){
    /*! Reset the data to the first line, end of the header file 
     *  Extract new block of data
     */ 
    this->end_pos_ = this->header_end_pos_;
    this->end_data_= false;
    this->eof_ = false;
    this->current_line_index_ = 0;    
    this->current_block_line_ = 0;
    this->site_ = 0;
    this->chrom_ = 0;
    this->previous_site_at_ = 0;
    if ( !this->withdata() ){ this->empty_file_line_counter_ = 0;  }
    this->read_new_block();
    }
    
    
void Vcf::init(string infile_name, int buffer_length){
    /*! Initialize the Vcf class members
     */ 
    this->current_line_index_= 0;
    this->file_name_ = infile_name;
    this->withdata_ = false;
    this->empty_file_line_counter_ = 0;
    this->nsam_ = 0;
    this->nfield_ = 0;
    
    this->current_block_line_ = 0;
    this->site_ = 0;
    this->chrom_ = 0;
    this->previous_site_at_ = 0;
    this->buffer_max_number_of_lines = buffer_length;
    this->eof_=false;
    if ( file_name_.size() > 0 ){
        this->withdata_ = true;
    }
    this->vcf_length_ = 0;
    this->end_data_ = false;
    this->even_interval_ = 0.0;
}

void Vcf::read_new_line(){
    /*! Read Vcf data, extract mutation site and haplotype
     */ 
    current_line_index_++;
    
    alt.clear();
    vec_of_sample_alt.clear();
    vec_of_sample_alt_bool.clear();
    sample_alt.clear();
    phased.clear(); // True if it is phased, which has '|'
    
    if (!withdata()){
        empty_file_line_counter_ ++; 
        this->site_ = ( current_line_index_ == 1 ) ? 0 : this->site_ + even_interval_;
        //if (current_line_index_ == 1){
            //this->site_ = 0;
        //} else {
            //this->site_ = this->site_ + even_interval_;
        //}
        // add counter for empty file, first time calling read_new_line, site() = 0, second time, set site() = 100000, and haplotype  =  false  
        if (this->empty_file_line_counter_ > 10){
            this->end_data_=true;    
        }
        return;    
    }
    
    string line = this->buffer_lines[current_block_line_];
    skip=true;
    size_t feild_start=0;
    size_t feild_end=0;
    int counter=0;
    string tmp_str;
    while(feild_end<line.size()){
        feild_end=min(line.find('\t',feild_start),line.find('\n',feild_start)); 
        tmp_str=line.substr(feild_start,feild_end-feild_start);
        istringstream tmp_strm(tmp_str.c_str());
        switch(counter){
            case 0: {
                tmp_strm>>chrom_; 
            }
            break;
            
            case 1: {
                tmp_strm>>site_; 
                if (((site_ - previous_site_at_)<2) && (previous_site_at_ > 0) && (pervious_chrom_ == chrom_)){
                    cout << "Skip reads at chrom " << chrom_<<" at position " <<  site_<<", due to it's too close to the previous variant (at " << previous_site_at_ <<")." << endl;
                    previous_site_at_ = site_;
                    current_block_line_++;
                    return; 
                }
            }
            break;

            case 3: {
                ref=tmp_str; 
                //if (ref.size()>1){
                //cout << "Skip reads at chrom " << chrom_<<" at position " <<  site_<<", due to deletion or replacement" << endl;
                //return;}
            }
            break;
            
            case 4: { 
                size_t alt_start=0;size_t alt_end=0; string alt_str;
                while (alt_end<tmp_str.size()){
                    alt_end=min(tmp_str.find(',',alt_start),tmp_str.size());
                    alt_str=tmp_str.substr(alt_start,alt_end);
                    //if (alt_str.size()>1){
                        //cout << "Skip reads at chrom " << chrom_<<" at position " <<  site_<<", due to insertion" << endl;
                        //return;}
                    alt.push_back(alt_str);
                    alt_start=alt_end+1;
                }
            }
            break;
            
            case 6: {
                if (tmp_str!="PASS"){
                    cout << "Skip reads at chrom " << chrom_<<" at position " <<  site_<<", due to low qualitiy." << endl;
                    current_block_line_++;
                    return;
                }
            }
            break;
        }

        if (counter > 8){
            size_t bar_index=tmp_str.find('|',0);        
            size_t slash_index=tmp_str.find('/',0);
            size_t colon_index=tmp_str.find(':',0);
            size_t break_index=min(bar_index, slash_index);
            assert(break_index<colon_index);
            vec_of_sample_alt.push_back(extract_alt_(tmp_str, 0, break_index));
            
            vec_of_sample_alt.push_back(extract_alt_(tmp_str, break_index+1, colon_index));
            
            istringstream alt_index_0_strm(tmp_str.substr(0,1));
            size_t alt_index_0;
            alt_index_0_strm>>alt_index_0;
            // use strtod to replace the previous three lines
            vec_of_sample_alt_bool.push_back( (alt_index_0 == 0)? false : true);
            //if (alt_index_0==0){
                //vec_of_sample_alt_bool.push_back(false);
            //} else {
                //vec_of_sample_alt_bool.push_back(true);
            //}
            
            istringstream alt_index_2_strm(tmp_str.substr(2,1));
            size_t alt_index_2;
            alt_index_2_strm>>alt_index_2;
            
            vec_of_sample_alt_bool.push_back( (alt_index_2 == 0)? false : true);
            //if (alt_index_2==0){
                //vec_of_sample_alt_bool.push_back(false);
            //} else {
                //vec_of_sample_alt_bool.push_back(true);
            //}
            }
        feild_start=feild_end+1;        
        counter++;
        }
    skip=false;
    // CHECK END of Vcf
    
    current_block_line_++;
    if (current_block_line_ == buffer_lines.size() ){ // END of the buff block
        if (!this->eof_){
            this->empty_block();
            this->read_new_block();
            }
        else{
            this->end_data_=true;    
            }
        }
    }


void Vcf::empty_block() { buffer_lines.clear(); }


void Vcf::read_new_block(){
    if (current_line_index_ == 0){
        cout << "Set data to the first entry, read a block of " <<  this->buffer_max_number_of_lines << " entries" <<endl;
        } 
    else {
        cout << "Read new block of " <<  this->buffer_max_number_of_lines << " entries, currently at line " << current_line_index_ << endl;
        }
    buffer_lines.clear();
    current_block_line_ = 0;

    if (!this->withdata_){
        this->site_ = 0;
        this->eof_=true;
        return;
        }

    ifstream infile (file_name_.c_str(), std::ifstream::binary);
    infile.seekg(this->end_pos_, ios::beg);
    int count=0;
    string tmp_str;
    
    getline(infile, tmp_str);
    
    while (infile.good() && tmp_str.size()>0 && count < buffer_max_number_of_lines){
        buffer_lines.push_back(tmp_str);
        this->end_pos_ += tmp_str.size()+1;
        count++;
        getline(infile, tmp_str);
        }
    
    if (vcf_length_ <= this->end_pos_){
        this->eof_=true;
        }
    
    infile.close();
    }

string Vcf::extract_alt_(string tmp_str, size_t start, size_t end){
    /*! Extract haplotype
     */ 
    string alt_index_str=tmp_str.substr(start,end-start);
    istringstream alt_index_strm(alt_index_str.c_str());
    size_t alt_index;
    alt_index_strm>>alt_index;
    
    string alt_dummy;
    alt_dummy = ( alt_index==0 ) ? ref : alt[alt_index-1];
    //if (alt_index==0){
        //alt_dummy=ref;
    //}
    //else{
        //alt_dummy=alt[alt_index-1];
    //}        
    return alt_dummy;
    }
    

void Vcf::print_vcf_line( vector<string> sample_names ){
    dout << "CHROM = " <<chrom_<<endl;
    dout << "POS = " << site_ <<endl;
    for (size_t i=0;i<sample_names.size();i++){
        dout << sample_names[i]<<": ";
        for (size_t j=0;j<sample_alt[i].size();j++){
            dout << sample_alt[i][j]<<" ";
            }  dout << endl;
        }  dout << endl;
    }


void Vcf::print(){
    for (size_t i = 0; i < vec_of_sample_alt_bool.size(); i++){
            cout << vec_of_sample_alt_bool[i]<<" ";
        } cout << endl;
    }


void Vcf::check_feilds(string line){
    size_t feild_start=0;
    size_t feild_end=0;
    int counter=0;
    string tmp_str;
    while( feild_end < line.size() ) {
        feild_end=min(line.find('\t',feild_start),line.find('\n',feild_start)); 
        tmp_str=line.substr(feild_start,feild_end-feild_start);
        switch (counter){
            case 0: if ( tmp_str != "#CHROM" ){ throw std::invalid_argument( "First Header entry should be #CHROM: "   + tmp_str ); } break;
            case 1: if ( tmp_str != "POS"    ){ throw std::invalid_argument( "Second Header entry should be POS: "     + tmp_str ); } break;
            case 2: if ( tmp_str != "ID"     ){ throw std::invalid_argument( "Third Header entry should be ID: "       + tmp_str ); } break;
            case 3: if ( tmp_str != "REF"    ){ throw std::invalid_argument( "Fourth Header entry should be REF: "     + tmp_str ); } break; 
            case 4: if ( tmp_str != "ALT"    ){ throw std::invalid_argument( "Fifth Header entry should be ALT: "      + tmp_str ); } break;
            case 5: if ( tmp_str != "QUAL"   ){ throw std::invalid_argument( "Sixth Header entry should be QUAL: "     + tmp_str ); } break;
            case 6: if ( tmp_str != "FILTER" ){ throw std::invalid_argument( "Seventh Header entry should be FILTER: " + tmp_str ); } break;
            case 7: if ( tmp_str != "INFO"   ){ throw std::invalid_argument( "Eighth Header entry should be INFO: "    + tmp_str ); } break;
            case 8: if ( tmp_str != "FORMAT" ){ throw std::invalid_argument( "Ninth Header entry should be FORMAT: "   + tmp_str ); } break;
            }
        
        if (counter > 8){  sample_names.push_back(tmp_str); }
        
        feild_start=feild_end+1;
        counter++;
        } // End of while loop: feild_end < line.size()
    
    nfield_=counter;
    set_nsam(int(sample_names.size()));
    assert(print_sample_name());   
    
    }


bool Vcf::print_sample_name(){
    dout << "Sample names:" << endl;
    for (size_t i = 0; i < this->nsam(); i++){
        dout << sample_names[i]<<" ";
        }  dout << endl;
    return true;
    }