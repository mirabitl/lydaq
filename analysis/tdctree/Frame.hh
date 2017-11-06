#ifndef _FRAME_HH
#define  _FRAME_HH
/**
   \class Frame
   \brief access to pad hit info from an uint64_t coding

 */ 
class Frame
  {
  public:
    /*!
     *  \brief Constructor
     *
     *  \param f : uint64_t encoding the hit result
     */
    Frame(uint64_t f){_frame=f;}
    Frame(){_frame=0;}
    /*!
     *  \brief Constructor
     *
     *  Encoding 
     *  \param dif : DIF number
     *  \param asic : HR2 number
     *  \param channel : ASIC channel number
     *  \param threshold : 0,1,2
     *  \param bc :  32 bit time shift to the readout in 200 ns unit
     */
    Frame(uint8_t dif,uint8_t asic,uint8_t channel,uint8_t threshold,uint32_t bc){ this->set(dif,asic,channel,threshold,bc);}
    //! @return DIF number
    inline uint32_t dif(){return (_frame&0xFF);}
    //! @return ASIC number
    inline uint32_t asic( ){return ((_frame>>8)&0x3F);}
    //! @return AISC channel
    inline uint32_t channel( ){return ((_frame>>14)&0x3F);}
    //! @return threhsold (0,1,2)
    inline uint32_t threshold(){return ((_frame>>20)&0x3);}
    //! @return 32 bit time shift to the readout in 200 ns unit
    inline uint32_t bc(){ return ((_frame>>32)&0xFFFFFFFF);}
    /**
     * encoding
     *  \param dif : DIF number
     *  \param asic : HR2 number
     *  \param channel : ASIC channel number
     *  \param threshold : 0,1,2
     *  \param bc :  32 bit time shift to the readout in 200 ns unit
     */
    inline void set(uint8_t dif,uint8_t asic,uint8_t channel,uint8_t threshold,uint32_t bc)
    {
      uint64_t d=dif,a=asic,c=channel,t=threshold,b=bc;
      _frame=((b&0xFFFFFFFF)<<32)|((t&0x3)<<20)|((c&0x3f)<<14)|((a&0x3F)<<8)|
        (d&0xFF);}
    //! @return  encoded value
    inline uint64_t value(){return _frame;}
    //! set  encoded value
    inline void set( uint64_t f){_frame=f;}
  private:
    uint64_t _frame;
  };
#endif
