
       SUBROUTINE BOUNDR1(ksix,ksiy,ksiz,IFI1,IFI)
           parameter (nksx=24,nksy=24,nksz=12,kfm=3604)
           parameter(nfi=80,mpsi=50,np=4,knp=nfi/np)
            parameter(iprread=0)
  
       
       parameter(isetka=49)   
       COMMON/BEGIN/     FZ1(knp,mpsi,KFM)
       COMMON/NFFR/      NFC1(nksx,nksy,nksz),NN
       COMMON/MASSIV/    iim(nksx),jjn(nksx,nksy)
       
       COMMON/param/     Tw,T_stop,U0
       common/bound/      aksixm1,aksixm2,aksiym1,aksiym2,aksiym
       COMMON/iter/    iter,id

       common/angle/     fi(isetka)
       COMMON/STEP/      h
       REAL*8        ksiy(nksy),ksiz(nksz),ksix(nksx),mom,mom1,ksiy_2,
     ,               bfm,b,aksiym1,aksiym2,aksiym,pi,api,h,
     ,  aksixm1,aksixm2
       INTEGER*2     NFC1,iim,jjn,NN
       real*8        U0,TW,T_stop,fz1
c...................................
           pi=dacos(0.0d0)*2.d0
           api=pi**1.5
           api0=api*T_stop**1.5
   
   

        
         ac=cos(fi(ifi1))
       bc=sin(fi(ifi1))
          mom=0.0d0
          mom1=0.0d0
        DO 7 iz=1,nksx
        DO 7 i=nksy-iim(iz)+1,iim(iz)
        DO 7 j=1,jjn(iz,i)

             ll=nfc1(iz,i,j)
             ksiy_2=ksiy(i)**2+ksix(iz)**2+ksiz(j)**2
             bfm=ksix(iz)*ac-ksiy(i)*bc
             IF(bfm.ge.0.0)THEN
               
               mom=FZ1(ifi,2,ll)*bfm+mom
 
              END IF
             IF(bfm.lt.0.0)THEN
             

        mom1=1.0/api0*dexp(-ksiy_2/T_stop)*bfm+mom1
 
              END IF
 7       CONTINUE

          DO 1 iz=1,nksx
          DO 1 i=nksy-iim(iz)+1,iim(iz)
          DO 1 j=1,jjn(iz,i)

          ll=nfc1(iz,i,j)
          
          ksiy_2=ksiy(i)**2+ksix(iz)**2+ksiz(j)**2
          bfm=ksix(iz)*cos(fi(ifi1))-ksiy(i)*sin(fi(ifi1))
          IF(bfm.lt.0.0)THEN
              

          FZ1(ifi,1,ll)=-mom/mom1/api0*dexp(-ksiy_2/T_stop)
   
                     
              
            END IF

 1        CONTINUE
        
                              
       
       
          RETURN
          END
   
c ********INTEGER nodes in one sphera
       INTEGER function asign(x)
       REAL*8 x
       asign=0
       IF(x.lT.0.0)asign=-1
       IF(x.gT.0.0)asign=1
       RETURN
       END
c******************************
       SUBROUTINE MOMENT(DENS)
        parameter (nksx=24,nksy=24,nksz=12,kfm=3604)
        parameter(nfi=80,mpsi=50,np=4,knp=nfi/np)
        parameter(iprread=0)
     
       REAL*8           h,FZ1
       real*8           DENS(knp,mpsi),fz
       COMMON/BEGIN/    FZ1(knp,mpsi,KFM)

       COMMON/STEP/     h
c.........................................................
       DO 1 k=1,KNP
       DO 1 l1=1,mpsi
       DENS(k,l1)=0.00
       DO 1 ll=1,KFM
       DENS(k,l1)=DENS(k,l1)+FZ1(k,l1,ll)*h*h*h*2
  1    CONTINUE
       RETURN
       END
c****************************************************************
c********************************************************
        SUBROUTINE CORXY
           parameter (nksx=24,nksy=24,nksz=12,kfm=3604)
           parameter(nfi=80,mpsi=50,np=4,knp=nfi/np)
           parameter(iprread=0)
 

        COMMON/gridd/Y(nfi,mpsi),X(nfi,mpsi)
          open(2,file='setka0.xy',status='unknown')
          do i=1,nfi
          do j=1,mpsi
          read(2,3)x(i,j),y(i,j)
            end do
             end do
      close(2)
3        FORMAT(2E12.4)
             
        RETURN

        END
c******************************************************
c.......................................................
       SUBROUTINE ckpt_main
      include'mpif.h'
      parameter (nksx=24,nksy=24,nksz=12,kfm=3604)
      parameter(nfi=80,mpsi=50,np=4,knp=nfi/np)
      parameter(iprread=0)
  
      parameter(isetka=49,isetka0=15,isetka1=29,isetka2=42)
      
      COMMON/BEGIN/   FZ1(knp,mpsi,KFM)
      COMMON/gridd/    y(nfi,mpsi),x(nfi,mpsi)
      
      COMMON/iter/    iter,id
     
      
      COMMON/inverse/ iu(KFM),jv(KFM),kw(KFM)
      
      COMMON/HELP/FNU(knp,mpsi,KFM),GEN(knp,mpsi,KFM),ADD1(knp,mpsi)
      COMMON/MASSIV/  IIM(nksx),JJN(nksx,nksy)
      COMMON/NFFR/    NFC1(nksx,nksy,nksz),NN
       COMMON/param/  Tw,T_stop,U0
       common/bound/   aksixm1,aksixm2,aksiym1,aksiym2,aksiym

      

      COMMON/STEP/    H
      COMMON/vel/     ksix(nksx),ksiy(nksy),ksiz(nksz)

   
       common/angle/fi(isetka)
      

      INTEGER*2       NFC1,iim,jjn,iu,jv,kw,izn,NN
      REAL*8 H,hy,hx,ksiy,ksiy_2,ksiz,
     ,ksix,pi,api,b,aksiym,aksiym1,aksiym2,aksixm1,aksixm2

       real*8 DENS(KNP,mpsi),
     ,u0,Tw,T_stop,m00(nfi,mpsi),m11(nfi,mpsi),
     ,u00(KFM),fz1,fnu,gen,add1,ffz,fij
       real*8 temp(knp,mpsi),
     *  xpsi(nfi,mpsi),xfi(nfi,mpsi),ypsi(nfi,mpsi),yfi(nfi,mpsi),
     *   GG(nfi,mpsi),akovmax(nfi,mpsi),flev,fdown,
     *   bkovmax(nfi,mpsi),FP(MPSI,KFM),FL(MPSI,KFM),F(MPSI,KFM)
          real*8 aas,aas1,as(nfi),fz0(knp,mpsi,kfm),ggg,astt
cc*************************************************
        CHARACTER*11,NAME,NAME2
                   
cc*************************************************
            INTEGER COUNT,DATATYPE,rank,TAG,COMM,
     *        STATUS(MPI_STATUS_SIZE),IERR,sendtype,root,size

          
cc***********************************************
c                CALL MPI_INIT(IERR)
            
               COMM=MPI_COMM_WORLD
              CALL MPI_COMM_SIZE(COMM,size,ierr)
              CALL MPI_COMM_RANK(COMM,rank,ierr)
                
               if(np.ne.size)then
               print*,'wrong value of np',np,size,id
               stop
               end if
            id=rank
 
             

c..............................................................
          ifin=100
          amax=3.0
      
          pi=2.d0*dacos(0.d0)
          gam=5.0/3.0
          Tw=1.0
          u0=amax*sqrt(5./6.*Tw)
       
           T_stop=4.0        
             print*,amax
c----------------------------------------------------------------------
c----------------------------------------------evaluation the bounds on
           aknud=1.0
           n1=5
           H=(u0)/(n1)
      
           nright=12
           nleft=nksx-nright
       aksixm2=nright*h
       aksixm1=nleft*h
       aksiym=(aksixm1+aksixm2)/2.00d0
            
        AKSIyM1=AKSIYM
        AKSIyM2=AKSIYM
                

c--------------------------------------------------------------------
c________________________________________correcition overfilling velocity
c                                        and calculating MACH number
        
            
                DO 9832 iz=1,nksx
                DO 9832 i=1,nksy
                DO 9832 j=1,nksz

 9832           NFC1(iz,i,j)=0
c.....................................................
c          determing of velocity grid
c*****************************************************
        DO 33 j=1,nksx
        ksix(j)=-aksiXm1+H/2.d0+H*(j-1)
33         continue
          
        DO 13 j=1,nksy/2
 13     ksiy(j)=-aksiym1+h/2.d0+h*(j-1)

        DO 23 j=nksy/2+1,nksy
 23     ksiy(j)=-ksiy(nksy+1-j)
         
   

        DO 3 j=1,nksz
  3     ksiz(j)=h/2.d0+h*(j-1)

c******* initial  condition**********
       DO 155 iz=1,nksx
       DO 155 i=nksy/2+1,nksy
       DO 155 j=1,nksz
            vc=(aksixm2-aksixm1)*0.5d0
c******determining of velocity sphere
       IF((ksiy(i))**2+ksiz(j)**2+(ksix(iz)-vc)**2.lt.aksiym**2)then
     

        IIM(iz)=i
        JJN(iz,i)=j
        JJN(iz,nksy+1-i)=j
        END IF
 155    CONTINUE

            ll1=0
          DO 137 iz=1,nksx
          DO 137 i=nksy-iim(iz)+1,nksy/2
          DO 137 j=1,jjn(iz,i)
           ll1=ll1+1
 137       NFC1(iz,i,j)=ll1
           NN=2*ll1
            
         
                             
          DO 138 iz=1,nksx
          DO 138 i=nksy-iim(iz)+1,nksy/2
          DO 138 j=1,jjn(iz,i)

138       NFC1(iz,nksy+1-i,j)=nn+1-NFC1(iz,i,j)
           
            call corxy
           in=isetka0-1
           in1=isetka1+1-isetka0
          
           in2=isetka2-(isetka1+1)
           in3=isetka-(isetka2+1)
          do ifi=1,isetka
          if(ifi.le.isetka0)then
           fi(ifi)=(ifi-1)*pi/4./in
           go to 45
           end if
          if(ifi.le.isetka1+1)then
          fi(ifi)=(ifi-isetka0)*pi/4./in1+pi/4.
          go to 45
           end if
         if(ifi.le.isetka2+1)then
           fi(ifi)=(ifi-(isetka1+1))*pi/4./in2+pi/2.
             go to 45
            end if
           if(ifi.le.isetka)then
           fi(ifi)=3.0/4.0*pi+pi/4.0*(ifi-(isetka2+1))/in3
      
           go to 45
            end if
45          continue   
              end do
            if(id.eq.1)print*,(fi(ifi),ifi=1,isetka)
                    

                  


                
c.................................................................
c......................................................................
                api=pi**(1.5d0)
           DO 55 ix=1,nksx
           DO 55 i=nksy-iim(ix)+1,iim(ix)
           DO 55 j=1,jjn(ix,i)
               ll=nfc1(ix,i,j)
               iu(ll)=ix
               jv(ll)=i
               kw(ll)=j
  55           CONTINUE
                

        DO 5 ll=1,kfm
        ix=iu(ll)
        i=jv(ll)
        j=kw(ll)
        ksiy_2=(ksiy(i)**2+(ksix(ix)-u0)**2+ksiz(j)**2)
        DO 5 lpsi=1,mpsi
        DO 5 lfi=1,knp
        FZ1(lfi,lpsi,ll)=1.0d0/api*dexp(-ksiy_2)
5       CONTINUE
c******************************
c*****************************
         CALL MOMENT(DENS)
               
        dnorm=dens(1,1)
      

c................................................
       DO 25 ll=1,KFM
       DO 25 lx=1,knp
       DO 25 ly=1,mpsi
       FZ1(lx,ly,ll)=FZ1(lx,ly,ll)/dnorm
       u00(ll)=FZ1(1,1,ll)
 25    CONTINUE
       
      
         Do i=1,nfi
         do j=1,mpsi
         if(i.eq.nfi)then
         xfi(i,j)=(x(i,j)-x(i-1,j))
         yfi(i,j)=(y(i,j)-y(i-1,j))
         else
         if (i.eq.1)then
         xfi(i,j)=(x(i+1,j)-x(i,j))
            yfi(i,j)=(y(i+1,j)-y(i,j))
          else
         xfi(i,j)=(x(i+1,j)-x(i-1,j))/2.0
           yfi(i,j)=(y(i+1,j)-y(i-1,j))/2.0
         end if
          end if

         if(j.eq.mpsi)then
         xpsi(i,j)=(x(i,j)-x(i,j-1))
           ypsi(i,j)=y(i,j)-y(i,j-1)
         else
         if (j.eq.1)then
         xpsi(i,j)=x(i,j+1)-x(i,j)
            ypsi(i,j)=y(i,j+1)-y(i,j)
          else
         xpsi(i,j)=(x(i,j+1)-x(i,j-1))/2.0
       
         ypsi(i,j)=(y(i,j+1)-y(i,j-1))/2.0
           end if
            end if
         GG(i,j)=abs(ypsi(i,j)*xfi(i,j)-xpsi(i,j)*yfi(i,j))
         
         
         end do
         end do
   
   
   
          iter=0

         Time=0
c..........determing inverse MASSIV to NFC1.........
          
              DO 73 lfi=1,nfi
              DO 73 lpsi=1,mpsi
              akovmax(lfi,lpsi)=0.0
             bkovmax(lfi,lpsi)=0.0
             DO 73 ix=1,nksx
              DO 73 i=nksy-IIM(ix)+1,IIM(ix)
            akov=abs(ksix(ix)*ypsi(lfi,lpsi)-ksiy(i)*xpsi(lfi,lpsi))
            bkov=abs(-ksix(ix)*yfi(lfi,lpsi)+ksiy(i)*xfi(lfi,lpsi))
              if(akov.gt.akovmax(lfi,lpsi))akovmax(lfi,lpsi)=akov  
           if(bkov.gt.bkovmax(lfi,lpsi))bkovmax(lfi,lpsi)=bkov
73            continue
             do  j=1,mpsi
         as(j)=1000.0
         do  i=1,nfi
       if(gg(i,j)/(akovmax(i,j)+bkovmax(i,j)).lt.as(j))as(j)=
     =    gg(i,j)/(akovmax(i,j)+bkovmax(i,j))
        end do
         end do
          
             name='VSAS000.dat'
             WRITE(name(5:7),'(I3.3)')id
               if(iprread.eq.1)then
          open(3,file=name,status='unknown',form='unformatted')
          read(3)(((FZ1(k,i,j),k=1,knp),i=1,mpsi),j=1,nn)
          close(3)
            end if
          

c!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
c!!!!!!!!!!BEGIN OF CYCLE!!!!!!!!!!!!!!!!!
1              CONTINUE
c***********************************
             DO IFI=1,KNP
             IFI1=ID*KNP+IFI
        IF(IFI1.ge.1.AND.IFI1.LE.ISETKA)CALl 
     *   BOUNDR1(ksix,ksiy,ksiz,IFI1,IFI)
        END DO
           
            
           

           

          DO 44 l=1,mpsi
          DO 44 kl=1,nn
          f(l,kl)=Fz1(knp,l,kl)


44        CONTINUE
c***********************************
          COUNT=mpsi*KFM
         DATATYPE=MPI_REAL8
          TAG=1

c*********first *send f recieve fl***********************
          if(id.ge.0.and.id.lt.np-1)
     *  CALL MPI_SEND(f,COUNT,DATATYPE,id+1,TAG,COMM,IERR)

         if(id.gt.0.and.id.le.np-1)
     *  CALL MPI_RECV(fl,COUNT,DATATYPE,id-1,TAG,COMM,STATUS,IERR)

c....................................................................

c....................................................................
             DO 145 l=1,mpsi
             DO 145 kl=1,nn
             f(l,kl)=Fz1(1,l,kl)
145           CONTINUE

             tag=2
c.............second send-receive................

          if(id.gt.0.and.id.le.np-1)
     *  CALL MPI_SEND(f,COUNT,DATATYPE,id-1,TAG,COMM,IERR)

          if(id.ge.0.and.id.lt.np-1)
     * CALL MPI_RECV(fp,COUNT,DATATYPE,id+1,TAG,COMM,STATUS,IERR)

            
              
           do 107 i=1,knp
           do 107 j=1,mpsi
           do 107 l=1,nn
           Fz0(i,j,l)=Fz1(i,j,l)
107        continue                             
               

             
c***************

              DO 72 ix=1,nksx
              DO 72 i=nksy-IIM(ix)+1,IIM(ix)
           
   
              DO 72 lfi=1,knp
                   lfi1=lfi+knp*id         
           
       
               DO 72 lpsi=1,mpsi
                   kkx=-1
                    
       akov=(ksix(ix)*ypsi(lfi1,lpsi)-ksiy(i)*
     *    xpsi(lfi1,lpsi))/GG(lfi1,lpsi)
        
        
            IF(akov.gt.0.0)kkx=1
                   kky=-1
   
      bkov=(-ksix(ix)*yfi(lfi1,lpsi)+ksiy(i)*
     *   xfi(lfi1,lpsi))/GG(lfi1,lpsi)
        
        
             

                 IF(bkov.gt.0.0)kky=1

                if(lfi1-kkx.eq.0.or.lfi1-kkx.eq.nfi+1)go to 88
              if(lpsi-kky.eq.0.or.lpsi-kky.eq.mpsi+1)go to 88
              DO 702 j=1,jjn(ix,i)
               kl=nfc1(ix,i,j)
   
               if(lfi-kkx.eq.0)then
                flev=fl(lpsi,kl)
            go to 765
                 end if

               if(lfi-kkx.eq.knp+1)then
                flev=fp(lpsi,kl)
                  else
               
                flev=FZ0(lfi-kkx,lpsi,kl)
                 end if
 765           continue                
                fdown=FZ0(lfi,lpsi-kky,kl)*bkov*kky
                 
             fij=FZ0(lfi,lpsi,kl)
         

             
c      tind0=0.8*gg(lfi1,lpsi)/
c     *       (akovmax(lfi1,lpsi)+bkovmax(lfi1,lpsi))
    
c           tind1=0.8/add1(lfi,lpsi)
        
c               tind=tind0*tind1/(tind0+tind1)
c                 tind=tind0
                 tind=as(lpsi)*0.8          
                flev=flev*kkx*akov

        ffz=fij*(1.0-kkx*akov*tind-kky*bkov*tind)+(flev+fdown)*tind
     
     
        FZ1(lfi,lpsi,kl)=ffz

               
              IF(ffz.lt.0.0)THEN
              print*,FZ1(lfi,lpsi,kl),lfi,lpsi,id
    
              stop
              END IF
              IF(ffz.gt.100.0)THEN
              print*,FZ1(lfi,lpsi,kl),lfi,lpsi,id
               stop
               end if
702            continue
88            continue
72            CONTINUE
70            CONTINUE
               
c****************************************************
              DO 79 ix=1,nksx
              DO 79 i=nksy-IIM(ix)+1,IIM(ix)
              DO 79 j=1,jjn(ix,i)
                 

                   kl=nfc1(ix,i,j)
                   if(ksiy(i).gt.0)then
                   do ipsi=1,mpsi
            if(id.eq.0)FZ1(1,ipsi,kl)=FZ1(1,ipsi,nn+1-kl)
                      end do

                    do ifi=1,knp
                     ifi1=id*knp+ifi
                      if(ifi1.ge.isetka.and.ifi1.le.nfi)then
                   FZ1(ifi,1,kl)=FZ1(ifi,1,nn+1-kl)
                    end if
                  end do
                  end if
   
 79                  continue   
                     iter=iter+1
                     if(id.eq.0)print*,iter
                       
C***************WRITING********************************
c******************************************************

          IF(iter/5*5.eq.iter)THEN
           
           
                 CALL MOMENT(DENS)
                  DO I=1,NFI
            DO J=1,MPSI
             M00(I,J)=0.0
             M11(I,J)=0.0
             END DO
             END DO

             DO I=1,KNP
             DO J=1,MPSI
           I1=ID*KNP+I
             M00(I1,J)=DENS(I,J)
             END DO
             END DO    
               
                                 root=2
                   count=MPSI*NFI
              sendtype=MPI_REAL8

            CALL MPI_REDUCE(M00,m11,count,
     *      sendtype,MPI_SUM,root,COMM,ierr)
             
          
              if(id.eq.root)then
      
             name2='DENC000.dat'
             WRITE(name2(5:7),'(I3.3)')iter/100

            
          
          open(3,file=name2,status='unknown')
          
          DO  I=1,NFI
          DO  j=1,MPSI
          WRITE(3,555)M11(I,J)
          end do
          end do                  
          close(3)
          end if
           END IF
 555   format(e12.4)          

c*****************************************

          if(iter/10*10.eq.iter)then
c          name='VSAS000.dat'
c             WRITE(name(5:7),'(I3.3)')id

c          open(3,file=name,status='unknown',form='unformatted')
c          write(3)(((FZ1(k,i,j),k=1,knp),i=1,mpsi),j=1,nn)
c          close(3)
          CALL ckptCheckpoint()
c          CALL ckptBenchmarkPrint()
          end if


c.............................................................
          if(iter.gt.ifin)go to 988
          GO TO 1
           
 
2207    format(' iter=',i5,'id=',i5)
2702    format(e12.4)
988      continue   
c        CALL MPI_FINALIZE(IERR)   
    
         STOP
         END

C ****************************************************************Sub. REL
C --- STAGE OF RELAXATION ---
          SUBROUTINE WRITE_GRID
c______________________  writing x,y coordinates in file 'grid.dat'_
           parameter (nksx=24,nksy=24,nksz=12,kfm=3604)
           parameter(nfi=80,mpsi=50,np=4,knp=nfi/np)
           parameter(iprread=1)
  
             common/gridd/y(nfi,mpsi),x(nfi,mpsi)
                  
          OPEN(2,file='cetkam.xy',status='unknown')

          DO  i=1,nfi
          DO  j=1,mpsi
          write(2,4) X(i,j),Y(i,j)
          END DO
          END DO
          CLOSE(2)

 4       format(2e12.4)
         RETURN
         END

C           
