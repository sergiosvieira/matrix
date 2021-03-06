/*
 * sparse_matrix2.cpp
 *
 *  Created on: 2014. 12. 29.
 *      Author: asran
 */

#include "sparse_matrix2.h"
#include "matrix_error.h"
#include <math.h>

#define	THREAD_NUM					(8)
#define	THREAD_FUNC_THRESHOLD	(THREAD_NUM)

namespace matrix
{

typedef	THREAD_RETURN_TYPE(THREAD_FUNC_TYPE *Operation)(void*);

struct		FuncInfo
{
	SparseMatrix2::OpInfo	opInfo;
	Operation				func;
	size_t					startCol;
	size_t					endCol;
};

/**
 * 생성자
 */
SparseMatrix2::SparseMatrix2			(	void	)
:mRowSize(0),
 mColSize(0),
 mData(NULL)
{
}

/**
 * 생성자
 */
SparseMatrix2::SparseMatrix2			(	size_t		row,	///< 행 크기
											size_t		col		///< 열 크기
										)
:mRowSize(0),
 mColSize(0),
 mData(NULL)
{
	allocElems(row, col);
}

/**
 * 복사 생성자
 */
SparseMatrix2::SparseMatrix2			(	const SparseMatrix2&		matrix		///< 복사 될 객체
										)
:mRowSize(0),
 mColSize(0),
 mData(NULL)
{
	allocElems(matrix.getRow(), matrix.getCol());
	copyElems(matrix);
}

/**
 * 소멸자
 */
SparseMatrix2::~SparseMatrix2			(	void	)
{
	freeElems();
}

/**
 * 행렬 요소 값 참조
 * @return 참조한 행렬 요소 값
 */
elem_t		SparseMatrix2::getElem		(	size_t		row,	///< 참조 할 행 위치
											size_t		col		///< 참조 할 열 위치
										) const
{
	//chkBound(row, col);

	elem_t		value	=	0;

	try
	{
		value	=	mData[row].mMap.at((unsigned int)col);
	}
	catch( std::out_of_range&	)
	{
		value	=	0;
	}

	return	value;
}

/**
 * 행렬 요소 값 설정
 */
void		SparseMatrix2::setElem			(	size_t		row,	///< 설정 할 행 위치
												size_t		col,	///< 설정 할 열 위치
												elem_t		elem	///< 설정 할 요소 값
											)
{
	//chkBound(row, col);

	if( elem != 0 )
	{
		mData[row].mMap[col]	=	elem;
	}
	else
	{
		mData[row].mMap.erase(col);
	}
}

/**
 * 행렬 덧셈
 * @return 행렬 덧셈 결과
 */
SparseMatrix2	SparseMatrix2::add		(	const SparseMatrix2&	operand	///< 피연산자
										) const
{
	chkSameSize(operand);

	SparseMatrix2	result		=	SparseMatrix2(getRow(), getCol());

	result		=	*this;

	for(size_t row=0;row<operand.getRow();++row)
	{
		for(elem_map_itor itor=operand.mData[row].mMap.begin();itor!=operand.mData[row].mMap.end();++itor)
		{
			result.setElem	(	row,
								itor->first,
								result.mData[row].mMap[itor->first] + itor->second
							);
		}
	}

	return	result;
}

/**
 * 쓰레드 행렬 덧셈
 * @return 행렬 덧셈 결과
 */
SparseMatrix2	SparseMatrix2::padd		(	const SparseMatrix2&	operand	///< 피연산자
										) const
{
	chkSameSize(operand);

	SparseMatrix2	result		=	SparseMatrix2(getRow(), getCol());

	if( getRow() < THREAD_FUNC_THRESHOLD )
	{
		result		=	*this;

		for(size_t row=0;row<operand.getRow();++row)
		{
			for(elem_map_itor itor=operand.mData[row].mMap.begin();itor!=operand.mData[row].mMap.end();++itor)
			{
				result.setElem	(	row,
									itor->first,
									result.mData[row].mMap[itor->first] + itor->second
								);
			}
		}
	}
	else
	{
		OpInfo		info;

		info.operandA	=	this;
		info.operandB	=	&operand;
		info.result		=	&result;

		doThreadFunc(FUNC_ADD, info);
	}

	return	result;
}

/**
 * 행렬 뺄셈
 * @return 행렬 뺄셈 결과
 */
SparseMatrix2	SparseMatrix2::sub		(	const SparseMatrix2&	operand	///< 피연산자
										) const
{
	chkSameSize(operand);

	SparseMatrix2	result		=	SparseMatrix2(getRow(), getCol());

	result		=	*this;

	for(size_t row=0;row<operand.getRow();++row)
	{
		for(elem_map_itor itor=operand.mData[row].mMap.begin();itor!=operand.mData[row].mMap.end();++itor)
		{
			result.setElem	(	row,
								itor->first,
								result.mData[row].mMap[itor->first] - itor->second
							);
		}
	}

	return	result;
}

/**
 * 쓰레드 행렬 뺄셈
 * @return 행렬 뺄셈 결과
 */
SparseMatrix2	SparseMatrix2::psub		(	const SparseMatrix2&	operand	///< 피연산자
										) const
{
	chkSameSize(operand);

	SparseMatrix2	result		=	SparseMatrix2(getRow(), getCol());

	if( getRow() < THREAD_FUNC_THRESHOLD )
	{
		result		=	*this;

		for(size_t row=0;row<operand.getRow();++row)
		{
			for(elem_map_itor itor=operand.mData[row].mMap.begin();itor!=operand.mData[row].mMap.end();++itor)
			{
				result.setElem	(	row,
									itor->first,
									result.mData[row].mMap[itor->first] - itor->second
								);
			}
		}
	}
	else
	{
		OpInfo		info;

		info.operandA	=	this;
		info.operandB	=	&operand;
		info.result		=	&result;

		doThreadFunc(FUNC_SUB, info);
	}

	return	result;
}

/**
 * 행렬 곱셈
 * @return 행렬 곱셈 결과
 */
SparseMatrix2	SparseMatrix2::multiply		(	const SparseMatrix2&	operand	///< 피연산자
											) const
{
	if( ( getRow() != operand.getCol() ) &&
		( getCol() != operand.getRow() ) )
	{
		throw	matrix::ErrMsg::createErrMsg("행렬 크기가 올바르지 않습니다.");
	}

	SparseMatrix2	result	=	SparseMatrix2(getRow(), operand.getCol());

	for(size_t row=0;row<getRow();++row)
	{
		for(elem_map_itor itor=mData[row].mMap.begin();itor!=mData[row].mMap.end();++itor)
		{
			for(elem_map_itor itor2=operand.mData[itor->first].mMap.begin();itor2!=operand.mData[itor->first].mMap.end();itor2++)
			{
				result.setElem	(	row,
									itor2->first,
									result.getElem(row, itor2->first) + (itor->second * itor2->second)
								);
			}
		}
	}

	return	result;
}

/**
 * 쓰레드 행렬 곱셈
 * @return 행렬 곱셈 결과
 */
SparseMatrix2	SparseMatrix2::pmultiply	(	const SparseMatrix2&	operand
											) const
{
	if( ( getRow() != operand.getCol() ) &&
		( getCol() != operand.getRow() ) )
	{
		throw	matrix::ErrMsg::createErrMsg("행렬 크기가 올바르지 않습니다.");
	}

	SparseMatrix2	result	=	SparseMatrix2(getRow(), operand.getCol());

	if( getRow() < THREAD_FUNC_THRESHOLD )
	{
		for(size_t row=0;row<getRow();++row)
		{
			for(elem_map_itor itor=mData[row].mMap.begin();itor!=mData[row].mMap.end();++itor)
			{
				for(elem_map_itor itor2=operand.mData[itor->first].mMap.begin();itor2!=operand.mData[itor->first].mMap.end();itor2++)
				{
					result.setElem	(	row,
										itor2->first,
										result.getElem(row, itor2->first) + (itor->second * itor2->second)
									);
				}
			}
		}
	}
	else
	{
		OpInfo		info;

		info.operandA	=	this;
		info.operandB	=	&operand;
		info.result		=	&result;

		doThreadFunc(FUNC_MULTIPLY, info);
	}

	return	result;
}

/**
 * 행렬 곱셈
 * @return 행렬 곱셈 결과
 */
SparseMatrix2	SparseMatrix2::multiply		(	elem_t		operand	///< 피연산자
											) const
{
	SparseMatrix2	result	=	SparseMatrix2(getRow(), getCol());

	for(size_t row=0;row<getRow();++row)
	{
		for(elem_map_itor itor=mData[row].mMap.begin();itor!=mData[row].mMap.end();++itor)
		{
			result.setElem	(	row,
									itor->first,
									itor->second * operand
								);
		}
	}

	return	result;
}

/**
 * 쓰레드 행렬 곱셈
 * @return 행렬 곱셈 결과
 */
SparseMatrix2	SparseMatrix2::pmultiply	(	elem_t		operand	///< 피연산자
											) const
{
	SparseMatrix2	result	=	SparseMatrix2(getRow(), getCol());

	if( getRow() < THREAD_FUNC_THRESHOLD )
	{
		for(size_t row=0;row<getRow();++row)
		{
			for(elem_map_itor itor=mData[row].mMap.begin();itor!=mData[row].mMap.end();++itor)
			{
				result.setElem	(	row,
										itor->first,
										itor->second * operand
									);
			}
		}
	}
	else
	{
		OpInfo		info;

		info.operandA		=	this;
		info.elemOperandB	=	operand;
		info.result			=	&result;

		doThreadFunc(FUNC_ELEM_MUL, info);
	}

	return	result;
}

/**
 * 전치 행렬 변환 후 곱셈
 * @return 행렬 곱셈 결과
 */
SparseMatrix2	SparseMatrix2::tmultiply	(	const SparseMatrix2&	operand	///< 피연산자
											) const
{
	if( ( getRow() != operand.getRow() ) &&
		( getCol() != operand.getCol() ) )
	{
		throw	matrix::ErrMsg::createErrMsg("행렬 크기가 올바르지 않습니다.");
	}

	SparseMatrix2	result	=	SparseMatrix2(getRow(), operand.getCol());

	for(size_t row=0;row<getRow();++row)
	{
		for(elem_map_itor itor=mData[row].mMap.begin();itor!=mData[row].mMap.end();++itor)
		{
			for(elem_map_itor itor2=operand.mData[row].mMap.begin();itor2!=operand.mData[row].mMap.end();itor2++)
			{
				result.setElem	(	itor->first,
										itor2->first,
										result.getElem(itor->first, itor2->first) + (itor->second * itor2->second)
									);
			}
		}
	}

	return	result;
}

/**
 * 쓰레드 전치 행렬 변환 후 곱셈
 * @return 행렬 곱셈 결과
 */
SparseMatrix2	SparseMatrix2::ptmultiply	(	const SparseMatrix2&	operand	///< 피연산자
												) const
{
	if( ( getRow() != operand.getRow() ) &&
		( getCol() != operand.getCol() ) )
	{
		throw	matrix::ErrMsg::createErrMsg("행렬 크기가 올바르지 않습니다.");
	}

	SparseMatrix2	result	=	SparseMatrix2(getRow(), operand.getCol());

	if( getRow() < THREAD_FUNC_THRESHOLD )
	{
		for(size_t row=0;row<getRow();++row)
		{
			for(elem_map_itor itor=mData[row].mMap.begin();itor!=mData[row].mMap.end();++itor)
			{
				for(elem_map_itor itor2=operand.mData[row].mMap.begin();itor2!=operand.mData[row].mMap.end();itor2++)
				{
					result.setElem	(	itor->first,
											itor2->first,
											result.getElem(itor->first, itor2->first) + (itor->second * itor2->second)
										);
				}
			}
		}
	}
	else
	{
		OpInfo		info;

		info.operandA		=	this;
		info.operandB		=	&operand;
		info.result		=	&result;

		doThreadFunc(FUNC_TMULTIPLY, info);
	}

	return	result;
}

/**
 * 앞 전치 행렬 변환 후 곱셈
 * @return 행렬 곱셈 결과
 */
SparseMatrix2	SparseMatrix2::stmultiply	(	const SparseMatrix2&	operand	///< 피연산자
											) const
{
	if( ( getRow() != operand.getRow() ) &&
		( getCol() != operand.getCol() ) )
	{
		throw	matrix::ErrMsg::createErrMsg("행렬 크기가 올바르지 않습니다.");
	}

	SparseMatrix2	result	=	SparseMatrix2(getRow(), operand.getCol());

	for(size_t row=0;row<getRow();++row)
	{
		for(elem_map_itor itor=mData[row].mMap.begin();itor!=mData[row].mMap.end();++itor)
		{
			for(elem_map_itor itor2=operand.mData[row].mMap.begin();itor2!=operand.mData[row].mMap.end();itor2++)
			{
				result.setElem	(	itor->first,
										itor2->first,
										result.getElem(itor->first, itor2->first) + (itor->second * itor2->second)
									);
			}
		}
	}

	return	result;
}

/**
 * 쓰레드 앞 전치 행렬 변환 후 곱셈
 * @return 행렬 곱셈 결과
 */
SparseMatrix2	SparseMatrix2::pstmultiply	(	const SparseMatrix2&	operand	///< 피연산자
												) const
{
	if( ( getRow() != operand.getRow() ) &&
		( getCol() != operand.getCol() ) )
	{
		throw	matrix::ErrMsg::createErrMsg("행렬 크기가 올바르지 않습니다.");
	}

	SparseMatrix2	result	=	SparseMatrix2(getRow(), operand.getCol());

	if( getRow() < THREAD_FUNC_THRESHOLD )
	{
		for(size_t row=0;row<getRow();++row)
		{
			for(elem_map_itor itor=mData[row].mMap.begin();itor!=mData[row].mMap.end();++itor)
			{
				for(elem_map_itor itor2=operand.mData[row].mMap.begin();itor2!=operand.mData[row].mMap.end();itor2++)
				{
					result.setElem	(	itor->first,
											itor2->first,
											result.getElem(itor->first, itor2->first) + (itor->second * itor2->second)
										);
				}
			}
		}
	}
	else
	{
		OpInfo		info;

		info.operandA		=	this;
		info.operandB		=	&operand;
		info.result		=	&result;

		doThreadFunc(FUNC_STMULTIPLY, info);
	}

	return	result;
}

/**
 * 행렬 대입
 * @return 대입 할 행렬
 */
const SparseMatrix2&		SparseMatrix2::equal			(	const SparseMatrix2&	operand	///< 피연산자
															)
{
	try
	{
		chkSameSize(operand);
		copyElems(operand);
	}
	catch( ErrMsg*	)
	{
		freeElems();
		allocElems(operand.getRow(), operand.getCol());
		copyElems(operand);
	}

	return	*this;
}

/**
 * 쓰레드 행렬 대입
 * @return 대입 할 행렬
 */
const SparseMatrix2&		SparseMatrix2::pequal		(	const SparseMatrix2&	operand	///< 피연산자
														)
{
	try
	{
		chkSameSize(operand);
		pcopyElems(operand);
	}
	catch( ErrMsg*	)
	{
		freeElems();
		allocElems(operand.getRow(), operand.getCol());
		pcopyElems(operand);
	}

	return	*this;
}

/**
 * 행렬 비교
 * @return 두 행렬이 같으면 true, 다르면 false 리턴
 */
bool			SparseMatrix2::compare		(	const SparseMatrix2&	operand	///< 피연산자
											) const
{
	bool	ret		=	true;

	if( getSize() == operand.getSize() )
	{
		for(size_t row=0;row<getRow();++row)
		{
			for(elem_map_itor itor=mData[row].mMap.begin();itor!=mData[row].mMap.end();++itor)
			{
				if( itor->second != operand.getElem(row, itor->first) )
				{
					ret		=	false;
					break;
				}
			}

			if( ret == false )
			{
				break;
			}
		}
	}
	else
	{
		ret		=	false;
	}

	return	ret;
}

/**
 * 쓰레드 행렬 비교
 * @return 두 행렬이 같으면 true, 다르면 false 리턴
 */
bool			SparseMatrix2::pcompare		(	const SparseMatrix2&	operand
												) const
{
	bool	ret		=	true;

	if( getSize() == operand.getSize() )
	{
		if( getRow() < THREAD_FUNC_THRESHOLD )
		{
			for(size_t row=0;row<getRow();++row)
			{
				for(elem_map_itor itor=mData[row].mMap.begin();itor!=mData[row].mMap.end();++itor)
				{
					if( itor->second != operand.getElem(row, itor->first) )
					{
						ret		=	false;
						break;
					}
				}

				if( ret == false )
				{
					break;
				}
			}
		}
		else
		{
			OpInfo		info;

			info.operandA		=	this;
			info.operandB		=	&operand;

			doThreadFunc(FUNC_COMPARE, info);

			ret		=	(info.retVal == 0)?(false):(true);
		}
	}
	else
	{
		ret		=	false;
	}

	return	ret;
}

/**
 * 행렬 방정식 해 계산
 * @return 해 계산 결과
 */
SparseMatrix2		SparseMatrix2::sol_cg		(	const SparseMatrix2&	operand	///< 피연산자
												)
{
	SparseMatrix2		x			=	SparseMatrix2(this->getCol(), operand.getCol());
	SparseMatrix2		r			=	operand - ( (*this) * x );
	SparseMatrix2		p			=	r;
	SparseMatrix2		rSold		=	r.tmultiply(r);
	SparseMatrix2		result		=	x;
	elem_t		min			=	1000;
	bool		foundFlag	=	false;

	for(size_t cnt=0;cnt<1000000;cnt++)
	{
		SparseMatrix2	ap		=	(*this) * p;
		elem_t			alpha	=	rSold.getElem(0,0) / (p.tmultiply(ap)).getElem(0,0);

		x	=	x + (p * alpha);
		r	=	r - (ap * alpha);

		SparseMatrix2	rsNew	=	r.tmultiply(r);

		elem_t		sqrtVal	=	sqrt(rsNew.getElem(0,0));

		if( min > sqrtVal )
		{
			min		=	sqrtVal;
			result	=	x;
		}

		if( sqrtVal < 0.001 )
		{
			foundFlag	=	true;
			break;
		}

		p		=	r + ( p * (rsNew.getElem(0,0) / rSold.getElem(0,0) ) );
		rSold	=	rsNew;
	}

	if( foundFlag != true )
	{
		x	=	result;
	}

	return	x;
}

/**
 * 행렬 데이터 공간 할당
 * @exception 메모리 할당 실패 시 에러 발생
 */
void		SparseMatrix2::allocElems		(	size_t		row,	///< 행 크기
												size_t		col		///< 열 크기
											)
{
	try
	{
		mRowSize	=	row;
		mColSize	=	col;

		mData	=	new map_node_t[row];
	}
	catch (	std::bad_alloc&	exception		)
	{
		throw matrix::ErrMsg::createErrMsg(exception.what());
	}
}

/**
 * 행렬 데이터 공간 할당 해제
 */
void		SparseMatrix2::freeElems		(	void	)
{
	delete[]	mData;
	mRowSize	=	0;
	mColSize	=	0;
}

/**
 * 행렬 데이터 복사
 */
void		SparseMatrix2::copyElems		(	const SparseMatrix2&		matrix		///< 복사 할 행렬
											)
{
	for(size_t row=0;row<getRow();++row)
	{
		mData[row].mMap.clear();
		mData[row]		=	matrix.mData[row];
	}
}

/**
 * 쓰레드 행렬 데이터 복사
 */
void		SparseMatrix2::pcopyElems		(	const SparseMatrix2&		matrix		///< 복사 할 행렬
												)
{
	if( getRow() < THREAD_FUNC_THRESHOLD )
	{
		for(size_t row=0;row<getRow();++row)
		{
			mData[row].mMap.clear();
			mData[row]		=	matrix.mData[row];
		}
	}
	else
	{
		OpInfo		info;

		info.operandA		=	this;
		info.operandB		=	&matrix;

		doThreadFunc(FUNC_COPY, info);
	}
}

/**
 * 같은 크기의 행렬인지 검사
 * @exception 행렬이 같은 크기가 아닐 경우 예외 발생
 */
void		SparseMatrix2::chkSameSize		(	const SparseMatrix2&		matrix		///< 비교 할 행렬
											) const
{
	if( ( getRow() != matrix.getRow() ) ||
		( getCol() != matrix.getCol() ) )
	{
		throw matrix::ErrMsg::createErrMsg("행렬 크기가 올바르지 않습니다.");
	}
}

/**
 * 행렬 요소 참조 범위 검사
 * @exception 참조 범위 밖일 경우 예외 발생
 */
void		SparseMatrix2::chkBound			(	size_t		row,	///< 참조 할 행 위치
												size_t		col		///< 참조 할 열 위치
											) const
{
	if( ( row >= getRow() ) ||
		( col >= getCol() ) )
	{
		throw	matrix::ErrMsg::createErrMsg("범위를 넘어서는 참조입니다.");
	}
}

void		SparseMatrix2::doThreadFunc		(	FuncKind	kind,
												OpInfo&		info
											) const
{
	FuncInfo		orgFuncInfo	=	{info, NULL, 0, 0};
	FuncInfo		funcInfo[THREAD_NUM];

#if(PLATFORM == PLATFORM_WINDOWS)

	HANDLE			id[THREAD_NUM];

#elif(PLATFORM == PLATFORM_LINUX)

	pthread_t		id[THREAD_NUM];

#endif

	switch( kind )
	{
	case FUNC_ADD:
		orgFuncInfo.func	=	SparseMatrix2::threadAdd;
		break;
	case FUNC_SUB:
		orgFuncInfo.func	=	SparseMatrix2::threadSub;
		break;
	case FUNC_MULTIPLY:
		orgFuncInfo.func	=	SparseMatrix2::threadMultiply;
		break;
	case FUNC_ELEM_MUL:
		orgFuncInfo.func	=	SparseMatrix2::threadElemMul;
		break;
	case FUNC_TMULTIPLY:
		orgFuncInfo.func	=	SparseMatrix2::threadTmultiply;
		break;
	case FUNC_STMULTIPLY:
		orgFuncInfo.func	=	SparseMatrix2::threadStmultiply;
		break;
	case FUNC_COMPARE:
		orgFuncInfo.func	=	SparseMatrix2::threadCompare;
		break;
	default:
		break;
	}

	size_t		threadPerCol	=	getRow() / THREAD_NUM;
	size_t		colMod			=	getRow() % THREAD_NUM;

	for(size_t num=0;num<THREAD_NUM;num++)
	{
		funcInfo[num]	=	orgFuncInfo;

		funcInfo[num].startCol	=	num * threadPerCol;
		funcInfo[num].endCol		=	funcInfo[num].startCol + threadPerCol - 1;
	}

	funcInfo[THREAD_NUM-1].endCol	+=	colMod;

	// Thread ??
	for(size_t num=0;num<THREAD_NUM;num++)
	{
#if(PLATFORM == PLATFORM_WINDOWS)

		id[num]	=	(HANDLE)_beginthreadex	(	NULL,
												0,
												SparseMatrix2::threadFunc,
												&funcInfo[num],
												0,
												NULL
											);

#elif(PLATFORM == PLATFORM_LINUX)

		pthread_create	(	&id[num],
								NULL,
								SparseMatrix2::threadFunc,
								&funcInfo[num]
							);

#endif

	}

	for(size_t num=0;num<THREAD_NUM;num++)
	{
		switch( kind )
		{
		case FUNC_COMPARE:
			{
				THREAD_RETURN_TYPE	retVal		=	(THREAD_RETURN_TYPE)FALSE;

				info.retVal		=	(THREAD_RETURN_TYPE)TRUE;

#if(PLATFORM == PLATFORM_WINDOWS)

				::WaitForSingleObjectEx	(	id[num],
											INFINITE,
											FALSE
										);

				::GetExitCodeThread(id[num], (LPDWORD)&retVal);

				::CloseHandle(id[num]);

#elif(PLATFORM == PLATFORM_LINUX)

				pthread_join(id[num], &retVal);

#endif

				info.retVal	=	(THREAD_RETURN_TYPE)((unsigned long)info.retVal & (unsigned long)retVal);
			}
			break;
		default:

#if(PLATFORM == PLATFORM_WINDOWS)

			::WaitForSingleObjectEx	(	id[num],
										INFINITE,
										FALSE
									);
			::CloseHandle(id[num]);

#elif(PLATFORM == PLATFORM_LINUX)

			pthread_join(id[num], NULL);

#endif

			break;
		}
	}
}

void		SparseMatrix2::doThreadFunc		(	FuncKind	kind,
												OpInfo&		info
											)
{
	FuncInfo		orgFuncInfo	=	{info, NULL, 0, 0};
	FuncInfo		funcInfo[THREAD_NUM];

#if(PLATFORM == PLATFORM_WINDOWS)

	HANDLE			id[THREAD_NUM];

#elif(PLATFORM == PLATFORM_LINUX)

	pthread_t		id[THREAD_NUM];

#endif

	switch( kind )
	{
	case FUNC_COPY:
		orgFuncInfo.func	=	SparseMatrix2::threadCopy;
		break;
	default:
		break;
	}

	size_t		threadPerCol	=	getRow() / THREAD_NUM;
	size_t		colMod			=	getRow() % THREAD_NUM;

	for(size_t num=0;num<THREAD_NUM;num++)
	{
		funcInfo[num]	=	orgFuncInfo;

		funcInfo[num].startCol	=	num * threadPerCol;
		funcInfo[num].endCol		=	funcInfo[num].startCol + threadPerCol - 1;
	}

	funcInfo[THREAD_NUM-1].endCol	+=	colMod;

	// Thread ??
	for(size_t num=0;num<THREAD_NUM;num++)
	{
#if(PLATFORM == PLATFORM_WINDOWS)

		id[num]	=	(HANDLE)_beginthreadex	(	NULL,
												0,
												SparseMatrix2::threadFunc,
												&funcInfo[num],
												0,
												NULL
											);

#elif(PLATFORM == PLATFORM_LINUX)

		pthread_create	(	&id[num],
								NULL,
								SparseMatrix2::threadFunc,
								&funcInfo[num]
							);

#endif
	}

	for(size_t num=0;num<THREAD_NUM;num++)
	{
#if(PLATFORM == PLATFORM_WINDOWS)

		::WaitForSingleObjectEx	(	id[num],
									INFINITE,
									FALSE
								);
		::CloseHandle(id[num]);

#elif(PLATFORM == PLATFORM_LINUX)

		pthread_join(id[num], NULL);

#endif
	}
}

THREAD_RETURN_TYPE THREAD_FUNC_TYPE	SparseMatrix2::threadFunc		(	void*	pData	)
{
	FuncInfo*	info	=	(FuncInfo*)pData;

	return	info->func(info);
}

THREAD_RETURN_TYPE THREAD_FUNC_TYPE	SparseMatrix2::threadAdd		(	void*	pData	)
{
	FuncInfo*	info	=	(FuncInfo*)pData;
	size_t		start	=	info->startCol;
	size_t		end		=	info->endCol;
	size_t		range	=	end - start;

	const SparseMatrix2&	operandA	=	*info->opInfo.operandA;
	const SparseMatrix2&	operandB	=	*info->opInfo.operandB;
	SparseMatrix2&			result		=	*info->opInfo.result;

	map_node_t*	nodeA	=	&operandA.mData[start];
	map_node_t*	nodeB	=	&operandB.mData[start];
	map_node_t*	nodeRet	=	&result.mData[start];

	for(size_t row=0;row<=range;++row)
	{
		for(elem_map_itor itor=nodeA[row].mMap.begin();itor!=nodeA[row].mMap.end();++itor)
		{
			nodeRet[row].mMap[itor->first]	=	itor->second;
		}
	}

	for(size_t row=0;row<=range;++row)
	{
		for(elem_map_itor itor=nodeB[row].mMap.begin();itor!=nodeB[row].mMap.end();++itor)
		{
			elem_t		val		=	nodeRet[row].mMap[itor->first] + itor->second;

			if( val != 0 )
			{
				nodeRet[row].mMap[itor->first]	=	val;
			}
			else
			{
				nodeRet[row].mMap.erase(itor->first);
			}
		}
	}

	return		NULL;
}

THREAD_RETURN_TYPE THREAD_FUNC_TYPE	SparseMatrix2::threadSub			(	void*	pData	)
{
	FuncInfo*	info	=	(FuncInfo*)pData;
	size_t		start	=	info->startCol;
	size_t		end		=	info->endCol;
	size_t		range	=	end - start;

	const SparseMatrix2&	operandA	=	*info->opInfo.operandA;
	const SparseMatrix2&	operandB	=	*info->opInfo.operandB;
	SparseMatrix2&			result		=	*info->opInfo.result;

	map_node_t*	nodeA	=	&operandA.mData[start];
	map_node_t*	nodeB	=	&operandB.mData[start];
	map_node_t*	nodeRet	=	&result.mData[start];

	for(size_t row=0;row<=range;++row)
	{
		for(elem_map_itor itor=nodeA[row].mMap.begin();itor!=nodeA[row].mMap.end();++itor)
		{
			nodeRet[row].mMap[itor->first]	=	itor->second;
		}
	}

	for(size_t row=0;row<=range;++row)
	{
		for(elem_map_itor itor=nodeB[row].mMap.begin();itor!=nodeB[row].mMap.end();++itor)
		{
			elem_t		val		=	nodeRet[row].mMap[itor->first] - itor->second;

			if( val != 0 )
			{
				nodeRet[row].mMap[itor->first]	=	val;
			}
			else
			{
				nodeRet[row].mMap.erase(itor->first);
			}
		}
	}

	return		NULL;
}

THREAD_RETURN_TYPE THREAD_FUNC_TYPE	SparseMatrix2::threadMultiply	(	void*	pData	)
{
	FuncInfo*	info	=	(FuncInfo*)pData;
	size_t		start	=	info->startCol;
	size_t		end		=	info->endCol;
	size_t		range	=	end - start;

	const SparseMatrix2&	operandA	=	*info->opInfo.operandA;
	const SparseMatrix2&	operandB	=	*info->opInfo.operandB;
	SparseMatrix2&			result		=	*info->opInfo.result;

	map_node_t*	nodeA	=	&operandA.mData[start];
	map_node_t*	nodeB	=	operandB.mData;
	map_node_t*	nodeRet	=	&result.mData[start];

	for(size_t row=0;row<=range;++row)
	{
		for(elem_map_itor itor=nodeA[row].mMap.begin();itor!=nodeA[row].mMap.end();++itor)
		{
			for(elem_map_itor itor2=nodeB[itor->first].mMap.begin();itor2!=nodeB[itor->first].mMap.end();itor2++)
			{
				try
				{
					elem_t		val		=	nodeRet[row].mMap[itor2->first] + (itor->second * itor2->second);

					if( val != 0 )
					{
						nodeRet[row].mMap[itor2->first]	=	val;
					}
					else
					{
						//printf("데이터가 0\n");
						nodeRet[row].mMap.erase(itor2->first);
					}
				}
				catch( std::out_of_range&	)
				{
					printf("범위 초과\n");
				}
			}
		}
	}

	return	NULL;
}

THREAD_RETURN_TYPE THREAD_FUNC_TYPE	SparseMatrix2::threadElemMul		(	void*	pData	)
{
	FuncInfo*	info	=	(FuncInfo*)pData;
	size_t		start	=	info->startCol;
	size_t		end		=	info->endCol;
	size_t		range	=	end - start;

	const SparseMatrix2&	operandA	=	*info->opInfo.operandA;
	elem_t					operandB	=	info->opInfo.elemOperandB;
	SparseMatrix2&			result		=	*info->opInfo.result;

	map_node_t*	nodeA	=	&operandA.mData[start];
	map_node_t*	nodeRet	=	&result.mData[start];

	for(size_t row=0;row<=range;++row)
	{
		for(elem_map_itor itor=nodeA[row].mMap.begin();itor!=nodeA[row].mMap.end();++itor)
		{
			try
			{
				elem_t		val			=	itor->second * operandB;

				if( val != 0 )
				{
					nodeRet[row].mMap[itor->first]	=	val;
				}
				else
				{
					//printf("데이터가 0\n");
					nodeRet[row].mMap.erase(itor->first);
				}
			}
			catch( std::out_of_range&	)
			{
				printf("범위 초과\n");
			}
		}
	}

	return	NULL;
}

THREAD_RETURN_TYPE THREAD_FUNC_TYPE	SparseMatrix2::threadTmultiply	(	void*	pData	)
{
	FuncInfo*	info	=	(FuncInfo*)pData;
	size_t		start	=	info->startCol;
	size_t		end		=	info->endCol;
	size_t		range	=	end - start;

	const SparseMatrix2&	operandA	=	*info->opInfo.operandA;
	const SparseMatrix2&	operandB	=	*info->opInfo.operandB;
	SparseMatrix2&			result		=	*info->opInfo.result;

	map_node_t*	nodeA	=	&operandA.mData[start];
	map_node_t*	nodeB	=	&operandB.mData[start];
	map_node_t*	nodeRet	=	result.mData;

	for(size_t row=0;row<=range;++row)
	{
		for(elem_map_itor itor=nodeA[row].mMap.begin();itor!=nodeA[row].mMap.end();++itor)
		{
			for(elem_map_itor itor2=nodeB[row].mMap.begin();itor2!=nodeB[row].mMap.end();itor2++)
			{
				LOCK(&nodeRet[itor->first].mLock);

				try
				{
					elem_t		val		=	nodeRet[itor->first].mMap[itor2->first] + (itor->second * itor2->second);

					if( val != 0 )
					{
						nodeRet[itor->first].mMap[itor2->first]	=	val;
					}
					else
					{
						//printf("데이터가 0\n");
						nodeRet[itor->first].mMap.erase(itor2->first);
					}
				}
				catch( std::out_of_range&	)
				{
					printf("범위 초과\n");
				}

				UNLOCK(&nodeRet[itor->first].mLock);
			}
		}
	}

	return	NULL;
}

THREAD_RETURN_TYPE THREAD_FUNC_TYPE	SparseMatrix2::threadStmultiply	(	void*	pData	)
{
	FuncInfo*	info	=	(FuncInfo*)pData;
	size_t		start	=	info->startCol;
	size_t		end		=	info->endCol;
	size_t		range	=	end - start;

	const SparseMatrix2&	operandA	=	*info->opInfo.operandA;
	const SparseMatrix2&	operandB	=	*info->opInfo.operandB;
	SparseMatrix2&			result		=	*info->opInfo.result;

	map_node_t*	nodeA	=	&operandA.mData[start];
	map_node_t*	nodeB	=	&operandB.mData[start];
	map_node_t*	nodeRet	=	result.mData;

	for(size_t row=0;row<=range;++row)
	{
		for(elem_map_itor itor=nodeA[row].mMap.begin();itor!=nodeA[row].mMap.end();++itor)
		{
			for(elem_map_itor itor2=nodeB[row].mMap.begin();itor2!=nodeB[row].mMap.end();itor2++)
			{
				LOCK(&nodeRet[itor->first].mLock);

				try
				{
					elem_t		val		=	nodeRet[itor->first].mMap[itor2->first] + (itor->second * itor2->second);

					if( val != 0 )
					{
						nodeRet[itor->first].mMap[itor2->first]	=	val;
					}
					else
					{
						//printf("데이터가 0\n");
						nodeRet[itor->first].mMap.erase(itor2->first);
					}
				}
				catch( std::out_of_range&	)
				{
					printf("범위 초과\n");
				}

				UNLOCK(&nodeRet[itor->first].mLock);
			}
		}
	}

	return	NULL;
}

THREAD_RETURN_TYPE THREAD_FUNC_TYPE	SparseMatrix2::threadCopy			(	void*	pData	)
{
	FuncInfo*	info	=	(FuncInfo*)pData;
	size_t		start	=	info->startCol;
	size_t		end		=	info->endCol;
	size_t		range	=	end - start;

	const SparseMatrix2&	operandA	=	*info->opInfo.operandA;
	const SparseMatrix2&	operandB	=	*info->opInfo.operandB;

	map_node_t*	nodeA	=	&operandA.mData[start];
	map_node_t*	nodeB	=	&operandB.mData[start];

	for(size_t row=0;row<=range;++row)
	{
		nodeA[row].mMap	=	nodeB[row].mMap;
	}

	return	NULL;
}

THREAD_RETURN_TYPE THREAD_FUNC_TYPE	SparseMatrix2::threadCompare		(	void*	pData	)
{
	THREAD_RETURN_TYPE	flag		=	(THREAD_RETURN_TYPE)TRUE;
	FuncInfo*			info		=	(FuncInfo*)pData;
	size_t				start		=	info->startCol;
	size_t				end			=	info->endCol;
	size_t				range		=	end - start;

	const SparseMatrix2&	operandA	=	*info->opInfo.operandA;
	const SparseMatrix2&	operandB	=	*info->opInfo.operandB;

	map_node_t*			nodeA		=	&operandA.mData[start];
	map_node_t*			nodeB		=	&operandB.mData[start];

	try
	{
		for(size_t row=0;row<=range;++row)
		{
			for(elem_map_itor itor=nodeA[row].mMap.begin();itor!=nodeA[row].mMap.end();++itor)
			{
				if( itor->second != nodeB[row].mMap.at(itor->first) )
				{
					flag	=	(THREAD_RETURN_TYPE)FALSE;
					break;
				}
			}

			if( flag == false )
			{
				break;
			}
		}
	}
	catch( std::out_of_range&	)
	{
		flag	=	(THREAD_RETURN_TYPE)FALSE;
	}

	return	(THREAD_RETURN_TYPE)flag;
}

};
