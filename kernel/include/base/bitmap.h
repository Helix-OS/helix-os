#ifndef _helix_bmap_h
#define _helix_bmap_h

#define BM_GET_BIT( m, i )	((m[i/8] & (1 << (i % 8)))? 1: 0 )
#define BM_SET_BIT( m, i, v )	(v?	(m[i/8] |=  (1 << (i % 8))):\
					(m[i/8] &= ~(1 << (i % 8))))

#endif
